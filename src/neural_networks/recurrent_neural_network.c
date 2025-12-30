/**
 * MIT License
 * Copyright (c) [2025] The Barfing Fox [Andrea Nardinocchi (andrea@nardinan.it)]
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#include <math.h>
#include "../../include/coremio/neural_networks/recurrent_neural_network.h"
int f_rnn_new(s_rnn *rnn, double learning_rate, double momentum_gradient, size_t time_frames, size_t layers, ...) {
  int result;
  memset(rnn, 0, sizeof(s_rnn));
  rnn->time_frames = time_frames;
  if ((result = p_fnn_container_new(&(rnn->core), learning_rate, momentum_gradient, layers)) == 0) {
    va_list arguments;
    va_start(arguments, layers);
    {
      for (size_t index_layer = 0; (index_layer < layers) && (result == 0); ++index_layer) {
        const size_t layer_neurons = va_arg(arguments, size_t);
        bool hidden_layer = ((index_layer > 0) && (index_layer < (layers - 1)));
        if (layer_neurons > 0)
          result = p_fnn_layer_new(&(rnn->core.layers[index_layer]), layer_neurons, time_frames, 0.0, hidden_layer,
            ((index_layer) ? &(rnn->core.layers[index_layer - 1]) : NULL));
      }
    }
    va_end(arguments);
    rnn->core.layers[rnn->core.elements_layers - 1].h_activator_function = p_fnn_softmax_function;
  }
  return result;
}
void f_rnn_dump_model(s_rnn *rnn, FILE *output_stream) {
  if ((rnn) && (output_stream)) {
    fprintf(output_stream, "%zu %f %f %f %zu %zu ", rnn->time_frames, rnn->smooth_loss, rnn->perplexity, rnn->accuracy,
      rnn->correct_predictions, rnn->total_predictions);
    f_fnn_dump_model(&(rnn->core), output_stream);
  }
}
int f_rnn_load_model(s_rnn *rnn, FILE *output_stream) {
  int result;
  if ((rnn) && (output_stream)) {
    fscanf(output_stream, "%zu %lf %lf %lf %zu %zu ", &(rnn->time_frames), &(rnn->smooth_loss), &(rnn->perplexity), &(rnn->accuracy),
      &(rnn->correct_predictions), &(rnn->total_predictions));
    if ((result = f_fnn_load_model(&(rnn->core), output_stream)) == 0)
      rnn->core.layers[rnn->core.elements_layers - 1].h_activator_function = p_fnn_softmax_function;
  } else
    result = 1;
  return result;
}
void f_rnn_free(s_rnn *rnn) {
  f_fnn_free(&(rnn->core));
}
static void p_rnn_forward_propagation_extended_input(s_rnn *rnn, const size_t *input) {
  if (input)
    for (size_t index_depth = 0; index_depth < rnn->time_frames; ++index_depth) {
      for (size_t index_input_neurons = 0; index_input_neurons < rnn->core.layers[0].elements_neurons; ++index_input_neurons)
        rnn->core.layers[0].neurons[index_input_neurons].values[index_depth].activator = 0.0;
      rnn->core.layers[0].neurons[input[index_depth]].values[index_depth].activator = 1.0;
    }
  for (size_t index_depth = 0; index_depth < rnn->time_frames; ++index_depth)
    for (size_t index_layer = 1; index_layer < rnn->core.elements_layers; ++index_layer) {
      bool hidden_layer = ((index_layer > 0) && (index_layer < (rnn->core.elements_layers - 1))),
        output_layer = (index_layer == (rnn->core.elements_layers - 1));
      for (size_t index_neuron = 0; index_neuron < rnn->core.layers[index_layer].elements_neurons; ++index_neuron) {
        /* start with bias */
        rnn->core.layers[index_layer].neurons[index_neuron].values[index_depth].activator_before_integral =
          rnn->core.layers[index_layer - 1].bias_to_next_layer[index_neuron];
        /* contribution from previous layer at same timestep (feedforward) */
        for (size_t index_neuron_previous_layer = 0;
          index_neuron_previous_layer < rnn->core.layers[index_layer - 1].elements_neurons; ++index_neuron_previous_layer) {
          rnn->core.layers[index_layer].neurons[index_neuron].values[index_depth].activator_before_integral +=
            (rnn->core.layers[index_layer - 1].neurons[index_neuron_previous_layer].values[index_depth].activator *
             rnn->core.layers[index_layer - 1].neurons[index_neuron_previous_layer].weights_to_next_layer.values[index_neuron]);
        }
        if ((index_depth > 0) && (hidden_layer))
          for (size_t index_neuron_previous_depth = 0;
            index_neuron_previous_depth < rnn->core.layers[index_layer].elements_neurons; ++index_neuron_previous_depth) {
            rnn->core.layers[index_layer].neurons[index_neuron].values[index_depth].activator_before_integral +=
              (rnn->core.layers[index_layer].neurons[index_neuron_previous_depth].values[index_depth - 1].activator *
               rnn->core.layers[index_layer].neurons[index_neuron_previous_depth].weights_to_same_layer.values[index_neuron]);
            }
      }
      if (output_layer) {
        double max_activation = rnn->core.layers[index_layer].neurons[0].values[index_depth].activator_before_integral;
        for (size_t index_neuron = 1; index_neuron < rnn->core.layers[index_layer].elements_neurons; ++index_neuron)
          if (rnn->core.layers[index_layer].neurons[index_neuron].values[index_depth].activator_before_integral > max_activation)
            max_activation = rnn->core.layers[index_layer].neurons[index_neuron].values[index_depth].activator_before_integral;
        rnn->core.layers[index_layer].accumulator_exp = 0;
        for (size_t index_neuron = 0; index_neuron < rnn->core.layers[index_layer].elements_neurons; ++index_neuron)
          rnn->core.layers[index_layer].accumulator_exp +=
            exp(rnn->core.layers[index_layer].neurons[index_neuron].values[index_depth].activator_before_integral - max_activation);
        for (size_t index_neuron = 0; index_neuron < rnn->core.layers[index_layer].elements_neurons; ++index_neuron) {
          const double shifted = rnn->core.layers[index_layer].neurons[index_neuron].values[index_depth].activator_before_integral - max_activation;
          rnn->core.layers[index_layer].neurons[index_neuron].values[index_depth].activator =
            exp(shifted) / rnn->core.layers[index_layer].accumulator_exp;
        }
      } else {
        for (size_t index_neuron = 0; index_neuron < rnn->core.layers[index_layer].elements_neurons; ++index_neuron)
          rnn->core.layers[index_layer].neurons[index_neuron].values[index_depth].activator =
            rnn->core.layers[index_layer].h_activator_function(&(rnn->core.layers[index_layer]),
              rnn->core.layers[index_layer].neurons[index_neuron].values[index_depth].activator_before_integral, false);
      }
    }
}
static void p_rnn_forward_propagation(s_rnn *rnn, const size_t *input) {
  if (rnn->core.elements_layers > 0)
    p_rnn_forward_propagation_extended_input(rnn, input);
}
static void p_rnn_back_propagation(s_rnn *rnn, const size_t *target_output) {
  if (rnn->core.elements_layers > 0) {
    rnn->core.recent_average_error = 0;
    for (size_t index_depth = 0; index_depth < rnn->time_frames; ++index_depth) {
      double cross_entropy_loss_at_depth = 0;
      for (size_t index_neuron = 0; index_neuron < rnn->core.layers[rnn->core.elements_layers - 1].elements_neurons; ++index_neuron) {
        double expected_value = ((target_output[index_depth] == index_neuron) ? 1.0 : 0.0),
          prediction = rnn->core.layers[rnn->core.elements_layers - 1].neurons[index_neuron].values[index_depth].activator,
          residual_neuron_at_timeframe = (prediction - expected_value);
        if (expected_value > 0.5)
          cross_entropy_loss_at_depth -= log(fmax(prediction, 1e-12));
        rnn->core.layers[rnn->core.elements_layers - 1].neurons[index_neuron].values[index_depth].error_responsibility_to_next_layer =
          residual_neuron_at_timeframe;
      }
      rnn->core.recent_average_error += cross_entropy_loss_at_depth;
      p_fnn_back_propagation_compute_local_gradient(&(rnn->core.layers[rnn->core.elements_layers - 1]), rnn->core.clipping_gradient, index_depth);
    }
    rnn->core.recent_average_error = (rnn->core.recent_average_error / (double)rnn->time_frames);
    if (rnn->smooth_loss == 0)
      rnn->smooth_loss = rnn->core.recent_average_error;
    else
      rnn->smooth_loss = d_rnn_smooth_loss_alpha * rnn->core.recent_average_error + (1.0 - d_rnn_smooth_loss_alpha) * rnn->smooth_loss;
    rnn->perplexity = exp(rnn->smooth_loss);
    for (int index_depth = (int)(rnn->time_frames - 1); index_depth >= 0; --index_depth)
      for (int index_layer = (int)(rnn->core.elements_layers - 2); index_layer >= 0; --index_layer) {
        p_fnn_back_propagation_calculate_bias_error_responsibility_to_next_layer_and_arc_corrections(&(rnn->core.layers[index_layer]),
          &(rnn->core.layers[index_layer + 1]), rnn->core.learning_rate, index_depth);
        p_fnn_back_propagation_compute_local_gradient(&(rnn->core.layers[index_layer]), rnn->core.clipping_gradient, index_depth);
      }
    for (int index_layer = (int)(rnn->core.elements_layers - 2); index_layer >= 0; --index_layer)
      p_fnn_back_propagation_update_arc_weights(&(rnn->core.layers[index_layer]), &(rnn->core.layers[index_layer + 1]),
        rnn->core.layers[index_layer + 1].elements_neurons, rnn->core.momentum_gradient, rnn->core.learning_rate, rnn->time_frames);
    ++rnn->core.learning_epoch;
  }
}
void f_rnn_run(s_rnn *rnn, const size_t *input, size_t *output_index, double *output_score) {
  if (rnn) {
    const size_t final_depth = rnn->time_frames - 1;
    p_rnn_forward_propagation(rnn, input);
    for (size_t index_layer = 1; index_layer < rnn->core.elements_layers; ++index_layer)
      for (size_t index_neuron = 0; index_neuron < rnn->core.layers[index_layer].elements_neurons; ++index_neuron) {
        rnn->core.layers[index_layer].neurons[index_neuron].values[0].activator =
          rnn->core.layers[index_layer].neurons[index_neuron].values[final_depth].activator;
        rnn->core.layers[index_layer].neurons[index_neuron].values[1].activator =
          rnn->core.layers[index_layer].neurons[index_neuron].values[final_depth].activator;
      }
    if ((output_index) && (output_score)) {
      *output_index = 0;
      *output_score = rnn->core.layers[rnn->core.elements_layers - 1].neurons[0].values[rnn->time_frames - 1].activator;
      for (size_t index = 1; index < rnn->core.layers[rnn->core.elements_layers - 1].elements_neurons; ++index)
        if (rnn->core.layers[rnn->core.elements_layers - 1].neurons[index].values[rnn->time_frames - 1].activator > *output_score) {
          *output_index = index;
          *output_score = rnn->core.layers[rnn->core.elements_layers - 1].neurons[index].values[(rnn->time_frames - 1)].activator;
        }
    }
  }
}
void f_rnn_train(s_rnn *rnn, const size_t *input, const size_t *target_output, double *optional_output_sequence) {
  if (rnn) {
    p_rnn_forward_propagation(rnn, input);
    p_rnn_back_propagation(rnn, target_output);
    {
      size_t predicted_index = 0;
      double highest_activation = rnn->core.layers[rnn->core.elements_layers - 1].neurons[0].values[rnn->time_frames - 1].activator;
      for (size_t index = 1; index < rnn->core.layers[rnn->core.elements_layers - 1].elements_neurons; ++index)
        if (rnn->core.layers[rnn->core.elements_layers - 1].neurons[index].values[rnn->time_frames - 1].activator > highest_activation) {
          predicted_index = index;
          highest_activation = rnn->core.layers[rnn->core.elements_layers - 1].neurons[index].values[rnn->time_frames - 1].activator;
        }
      if (predicted_index == target_output[rnn->time_frames - 1])
        ++(rnn->correct_predictions);
      ++(rnn->total_predictions);
      rnn->accuracy = ((double)rnn->correct_predictions / (double)rnn->total_predictions);
    }
    if (optional_output_sequence)
      for (size_t index_neuron = 0; index_neuron < rnn->core.layers[rnn->core.elements_layers - 1].elements_neurons; ++index_neuron)
        optional_output_sequence[index_neuron] =
          rnn->core.layers[rnn->core.elements_layers - 1].neurons[index_neuron].values[(rnn->time_frames - 1)].activator;
  }
}
void f_rnn_reset_state(s_rnn *rnn) {
  if (rnn)
    for (size_t index_layer = 0; index_layer < rnn->core.elements_layers; ++index_layer)
      for (size_t index_neuron = 0; index_neuron < rnn->core.layers[index_layer].elements_neurons; ++index_neuron)
        for (size_t index_depth = 0; index_depth < rnn->core.layers[index_layer].elements_depth; ++index_depth) {
          rnn->core.layers[index_layer].neurons[index_neuron].values[index_depth].activator = 0;
          rnn->core.layers[index_layer].neurons[index_neuron].values[index_depth].activator_before_integral = 0;
        }
}
static size_t p_rnn_sample_from_distribution(double *probs, size_t size, double temperature) {
  double sum = 0, cumulative = 0, random_value = (double)rand() / (double)RAND_MAX;
  size_t index;
  for (index = 0; index < size; ++index) {
    probs[index] = pow(fmax(probs[index], 1e-12), 1.0 / temperature);
    sum += probs[index];
  }
  for (index = 0; index < size; ++index)
    probs[index] /= sum;
  for (index = 0; index < size; ++index) {
    cumulative += probs[index];
    if (random_value <= cumulative)
      return index;
  }
  return (size - 1);
}
size_t f_rnn_generate_step(s_rnn *rnn, size_t input, double *output_probabilities, double temperature) {
  size_t result = 0;
  if (rnn) {
    const size_t current_depth = 0, previous_depth = 1;
    for (size_t index_layer = 1; index_layer < rnn->core.elements_layers; ++index_layer)
      for (size_t index_neuron = 0; index_neuron < rnn->core.layers[index_layer].elements_neurons; ++index_neuron)
        rnn->core.layers[index_layer].neurons[index_neuron].values[previous_depth].activator =
          rnn->core.layers[index_layer].neurons[index_neuron].values[current_depth].activator;
    for (size_t index_neuron = 0; index_neuron < rnn->core.layers[0].elements_neurons; ++index_neuron)
      rnn->core.layers[0].neurons[index_neuron].values[current_depth].activator = 0.0;
    if (input < rnn->core.layers[0].elements_neurons)
      rnn->core.layers[0].neurons[input].values[current_depth].activator = 1.0;
    for (size_t index_layer = 1; index_layer < rnn->core.elements_layers; ++index_layer) {
      bool hidden_layer = ((index_layer > 0) && (index_layer < (rnn->core.elements_layers - 1))),
        output_layer = (index_layer == (rnn->core.elements_layers - 1));
      for (size_t index_neuron = 0; index_neuron < rnn->core.layers[index_layer].elements_neurons; ++index_neuron) {
        rnn->core.layers[index_layer].neurons[index_neuron].values[current_depth].activator_before_integral =
          rnn->core.layers[index_layer - 1].bias_to_next_layer[index_neuron];
        for (size_t index_neuron_previous_layer = 0;
          index_neuron_previous_layer < rnn->core.layers[index_layer - 1].elements_neurons; ++index_neuron_previous_layer)
          rnn->core.layers[index_layer].neurons[index_neuron].values[current_depth].activator_before_integral +=
            (rnn->core.layers[index_layer - 1].neurons[index_neuron_previous_layer].values[current_depth].activator *
             rnn->core.layers[index_layer - 1].neurons[index_neuron_previous_layer].weights_to_next_layer.values[index_neuron]);
        if (hidden_layer)
          for (size_t index_neuron_previous_depth = 0;
            index_neuron_previous_depth < rnn->core.layers[index_layer].elements_neurons; ++index_neuron_previous_depth)
            rnn->core.layers[index_layer].neurons[index_neuron].values[current_depth].activator_before_integral +=
              (rnn->core.layers[index_layer].neurons[index_neuron_previous_depth].values[previous_depth].activator *
               rnn->core.layers[index_layer].neurons[index_neuron_previous_depth].weights_to_same_layer.values[index_neuron]);
      }
      if (output_layer) {
        double max_activation = rnn->core.layers[index_layer].neurons[0].values[current_depth].activator_before_integral;
        for (size_t index_neuron = 1; index_neuron < rnn->core.layers[index_layer].elements_neurons; ++index_neuron)
          if (rnn->core.layers[index_layer].neurons[index_neuron].values[current_depth].activator_before_integral > max_activation)
            max_activation = rnn->core.layers[index_layer].neurons[index_neuron].values[current_depth].activator_before_integral;
        rnn->core.layers[index_layer].accumulator_exp = 0;
        for (size_t index_neuron = 0; index_neuron < rnn->core.layers[index_layer].elements_neurons; ++index_neuron)
          rnn->core.layers[index_layer].accumulator_exp +=
            exp(rnn->core.layers[index_layer].neurons[index_neuron].values[current_depth].activator_before_integral - max_activation);
        for (size_t index_neuron = 0; index_neuron < rnn->core.layers[index_layer].elements_neurons; ++index_neuron) {
          double shifted = rnn->core.layers[index_layer].neurons[index_neuron].values[current_depth].activator_before_integral - max_activation;
          rnn->core.layers[index_layer].neurons[index_neuron].values[current_depth].activator =
            exp(shifted) / rnn->core.layers[index_layer].accumulator_exp;
        }
      } else {
        for (size_t index_neuron = 0; index_neuron < rnn->core.layers[index_layer].elements_neurons; ++index_neuron)
          rnn->core.layers[index_layer].neurons[index_neuron].values[current_depth].activator =
            rnn->core.layers[index_layer].h_activator_function(&(rnn->core.layers[index_layer]),
              rnn->core.layers[index_layer].neurons[index_neuron].values[current_depth].activator_before_integral, false);
      }
    }
    {
      const size_t output_size = rnn->core.layers[rnn->core.elements_layers - 1].elements_neurons;
      double *probabilities = (double *)d_malloc(sizeof(double) * output_size);
      if (probabilities) {
        for (size_t index = 0; index < output_size; ++index)
          probabilities[index] = rnn->core.layers[rnn->core.elements_layers - 1].neurons[index].values[current_depth].activator;
        result = p_rnn_sample_from_distribution(probabilities, output_size, temperature);
        if (output_probabilities)
          for (size_t index = 0; index < output_size; ++index)
            output_probabilities[index] = probabilities[index];
        d_free(probabilities);
      }
    }
  }
  return result;
}