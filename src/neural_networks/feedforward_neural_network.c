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
#include "../../include/coremio/neural_networks/feedforward_neural_network.h"
double p_fnn_hyperbolic_tangent_function(s_neural_layer *layer, const double value, const bool derivative) {
  return ((derivative) ? (1.0 - (value * value)) : tanh(value));
}
double p_fnn_sigmoid_function(s_neural_layer *layer, const double value, const bool derivative) {
  return ((derivative) ? (value * (1.0 - value)) : (1.0 / (1.0 + exp(-value))));
}
double p_fnn_rectifier_function(s_neural_layer *layer, const double value, const bool derivative) {
  return ((derivative) ? ((value < 0) ? 0 : ((value > 0) ? 1 : 0.5)) : ((value < 0) ? 0 : value));
}
double p_fnn_softmax_function(s_neural_layer *layer, double value, bool derivative) {
  return ((derivative) ? 1.0 : ((exp(value) / layer->accumulator_exp)));
}
double p_fnn_random_weight(const double elements_in_layer, const bool same_layer) {
  double range_limits = 0.1;
  if (!same_layer)
    range_limits = (1.0 / sqrt(elements_in_layer));
  return ((rand()/(RAND_MAX + 1.)) * (2.0 * range_limits)) - range_limits;
}
int p_fnn_layer_new(s_neural_layer *layer, const size_t elements_neurons, const size_t elements_depth, const double dropout_rate,
  const bool reentrant_arcs_needed, s_neural_layer *previous_layer) {
  int result = 0;
  layer->h_activator_function = p_fnn_hyperbolic_tangent_function; /* by default, we're using this one, but can be easily replaced */
  if ((layer->neurons = (s_neuron *)d_malloc(sizeof(s_neuron) * elements_neurons))) {
    memset(layer->neurons, 0, (sizeof(s_neuron) * elements_neurons));
    layer->elements_neurons = elements_neurons;
    for (size_t index_neuron = 0; (index_neuron < elements_neurons) && (result == 0); ++index_neuron)
      if (!(layer->neurons[index_neuron].values = (s_neuron_values *)d_malloc(sizeof(s_neuron_values) * elements_depth)))
        result = 1;
    if (result == 0) {
      layer->elements_depth = elements_depth;
      if ((elements_depth > 1) && (reentrant_arcs_needed)) /* we have more than one level of deepness, so we need to allocate our reentrant arcs */
        for (size_t index_neuron = 0; (index_neuron < elements_neurons) && (result == 0); ++index_neuron) {
          if (((layer->neurons[index_neuron].weights_to_same_layer.values = (double *)d_malloc(sizeof(double) * elements_neurons))) &&
              ((layer->neurons[index_neuron].weights_to_same_layer.corrections = (double *)d_malloc(sizeof(double) * elements_neurons))) &&
              ((layer->neurons[index_neuron].weights_to_same_layer.last_update = (double *)d_malloc(sizeof(double) * elements_neurons)))) {
            for (size_t index_weight = 0; index_weight < elements_neurons; ++index_weight) {
              layer->neurons[index_neuron].weights_to_same_layer.values[index_weight] = p_fnn_random_weight(elements_neurons, true);
              layer->neurons[index_neuron].weights_to_same_layer.last_update[index_weight] = 0;
            }
          } else
            result = 1;
        }
      if (result == 0)
        if (previous_layer) {
          if (((previous_layer->bias_to_next_layer = (double *)d_malloc(sizeof(double) * elements_neurons))) &&
            ((previous_layer->bias_corrections = (double *)d_malloc(sizeof(double) * elements_neurons))) &&
            ((previous_layer->bias_last_update = (double *)d_malloc(sizeof(double) * elements_neurons)))) {
            memset(previous_layer->bias_to_next_layer, 0, (sizeof(double) * elements_neurons));
            memset(previous_layer->bias_corrections, 0, (sizeof(double) * elements_neurons));
            memset(previous_layer->bias_last_update, 0, (sizeof(double) * elements_neurons));
            for (size_t index_neuron = 0; (index_neuron < previous_layer->elements_neurons) && (result == 0); ++index_neuron)
              if (((previous_layer->neurons[index_neuron].weights_to_next_layer.values = (double *)d_malloc(sizeof(double) * elements_neurons))) &&
                  ((previous_layer->neurons[index_neuron].weights_to_next_layer.corrections = (double *)d_malloc(sizeof(double) * elements_neurons))) &&
                  ((previous_layer->neurons[index_neuron].weights_to_next_layer.last_update = (double *)d_malloc(sizeof(double) * elements_neurons)))) {
                memset(previous_layer->neurons[index_neuron].weights_to_next_layer.corrections, 0, (sizeof(double) * elements_neurons));
                for (size_t index_weight = 0; index_weight < elements_neurons; ++index_weight) {
                  previous_layer->neurons[index_neuron].weights_to_next_layer.values[index_weight] =
                    p_fnn_random_weight(previous_layer->elements_neurons, false);
                  previous_layer->neurons[index_neuron].weights_to_next_layer.last_update[index_weight] = 0;
                }
              } else
                result = 1;
          } else
            result = 1;
        }
    }
  } else
    result = 1;
  return result;
}
int p_fnn_container_new(s_fnn *fnn, const double learning_rate, const double momentum_gradient, const size_t layers) {
  int result = 1;
  memset(fnn, 0, sizeof(s_fnn));
  fnn->clipping_gradient = d_fnn_clipping_gradient;
  if ((fnn->layers = (s_neural_layer *)d_malloc(sizeof(s_neural_layer) * layers))) {
    memset(fnn->layers, 0, (sizeof(s_neural_layer) * layers));
    fnn->learning_rate = learning_rate;
    fnn->momentum_gradient = momentum_gradient;
    fnn->elements_layers = layers;
    result = 0;
  }
  return result;
}
int f_fnn_new(s_fnn *fnn, const double learning_rate, const double momentum_gradient, const size_t layers, ...) {
  int result;
  if ((result = p_fnn_container_new(fnn, learning_rate, momentum_gradient, layers)) == 0) {
    va_list arguments;
    va_start(arguments, layers);
    {
      for (size_t index_layer = 0; (index_layer < layers) && (result == 0); ++index_layer) {
        const size_t layer_neurons = va_arg(arguments, size_t);
        if (layer_neurons > 0)
          result = p_fnn_layer_new(&(fnn->layers[index_layer]), layer_neurons, 1, 0.0, false, ((index_layer) ? &(fnn->layers[index_layer - 1]) : NULL));
      }
    }
    va_end(arguments);
  }
  return result;
}
void f_fnn_dump_model(s_fnn *fnn, FILE *output_stream) {
  if ((fnn) && (output_stream)) {
    fprintf(output_stream, "%zu %lf %lf ", fnn->elements_layers, fnn->learning_rate, fnn->momentum_gradient);
    for (size_t index_layer = 0; index_layer < fnn->elements_layers; ++index_layer) {
      fprintf(output_stream, "%zu %zu ", fnn->layers[index_layer].elements_neurons, fnn->layers[index_layer].elements_depth);
      if (fnn->layers[index_layer].bias_to_next_layer) {
        fprintf(output_stream, "%zu ", fnn->layers[index_layer + 1].elements_neurons);
        for (size_t index_bias = 0; index_bias < fnn->layers[index_layer + 1].elements_neurons; ++index_bias)
          fprintf(output_stream, "%lf ", fnn->layers[index_layer].bias_to_next_layer[index_bias]);
      } else
        fprintf(output_stream, "0 ");
      if (index_layer < (fnn->elements_layers - 1))
        for (size_t index_neuron = 0; index_neuron < fnn->layers[index_layer].elements_neurons; ++index_neuron) {
          if (fnn->layers[index_layer].neurons[index_neuron].weights_to_next_layer.values) {
            fprintf(output_stream, "%zu ", fnn->layers[index_layer + 1].elements_neurons);
            for (size_t index_neuron_next_layer = 0; index_neuron_next_layer < fnn->layers[index_layer + 1].elements_neurons; ++index_neuron_next_layer)
              fprintf(output_stream, "%lf ", fnn->layers[index_layer].neurons[index_neuron].weights_to_next_layer.values[index_neuron_next_layer]);
          } else
            fprintf(output_stream, "0 ");
        }
      if (fnn->layers[index_layer].elements_depth > 1)
        for (size_t index_neuron = 0; index_neuron < fnn->layers[index_layer].elements_neurons; ++index_neuron) {
          if (fnn->layers[index_layer].neurons[index_neuron].weights_to_same_layer.values) {
            fprintf(output_stream, "Y ");
            for (size_t index_neuron_next_depth = 0; index_neuron_next_depth < fnn->layers[index_layer].elements_neurons; ++index_neuron_next_depth)
              fprintf(output_stream, "%lf ", fnn->layers[index_layer].neurons[index_neuron].weights_to_same_layer.values[index_neuron_next_depth]);
          } else
            fprintf(output_stream, "N ");
        }
    }
  }
}
int f_fnn_load_model(s_fnn *fnn, FILE *output_stream) {
  int result;
  if ((fnn) && (output_stream)) {
    size_t layer_elements;
    double learning_rate, momentum_gradient;
    fscanf(output_stream, "%zu %lf %lf ", &layer_elements, &learning_rate, &momentum_gradient);
    if ((result = p_fnn_container_new(fnn, learning_rate, momentum_gradient, layer_elements)) == 0)
      for (size_t index_layer = 0; (index_layer < layer_elements) && (result == 0); ++index_layer) {
        size_t neuron_elements, depth_elements, bias_elements;
        fscanf(output_stream, "%zu %zu %zu ", &neuron_elements, &depth_elements, &bias_elements);
        if ((result = p_fnn_layer_new(&(fnn->layers[index_layer]), neuron_elements, depth_elements, 0.0, false, NULL)) == 0) {
          if (bias_elements > 0) {
            if (((fnn->layers[index_layer].bias_to_next_layer = (double *)d_malloc(sizeof(double) * bias_elements))) &&
              ((fnn->layers[index_layer].bias_corrections = (double *)d_malloc(sizeof(double) * bias_elements))) &&
              ((fnn->layers[index_layer].bias_last_update = (double *)d_malloc(sizeof(double) * bias_elements)))) {
              memset(fnn->layers[index_layer].bias_corrections, 0, (sizeof(double) * bias_elements));
              memset(fnn->layers[index_layer].bias_last_update, 0, (sizeof(double) * bias_elements));
              for (size_t index_bias = 0; index_bias < bias_elements; ++index_bias)
                fscanf(output_stream, "%lf ", &(fnn->layers[index_layer].bias_to_next_layer[index_bias]));
            } else
              result = 1;
          }
          if (result == 0) {
            if (index_layer < (fnn->elements_layers - 1))
              for (size_t index_neuron = 0; (index_neuron < neuron_elements) && (result == 0); ++index_neuron) {
                size_t neuron_next_layer_elements;
                fscanf(output_stream, "%zu ", &neuron_next_layer_elements);
                if (neuron_next_layer_elements > 0) {
                  size_t size = sizeof(double) * neuron_next_layer_elements;
                  if (((fnn->layers[index_layer].neurons[index_neuron].weights_to_next_layer.values = (double *)d_malloc(size))) &&
                      ((fnn->layers[index_layer].neurons[index_neuron].weights_to_next_layer.corrections = (double *)d_malloc(size))) &&
                      ((fnn->layers[index_layer].neurons[index_neuron].weights_to_next_layer.last_update = (double *)d_malloc(size)))) {
                    for (size_t index_neuron_next_layer = 0; index_neuron_next_layer < neuron_next_layer_elements; ++index_neuron_next_layer) {
                      fscanf(output_stream, "%lf ", &(fnn->layers[index_layer].neurons[index_neuron].weights_to_next_layer.values[index_neuron_next_layer]));
                      fnn->layers[index_layer].neurons[index_neuron].weights_to_next_layer.last_update[index_neuron_next_layer] = 0;
                    }
                  } else
                    result = 1;
                }
              }
            if (result == 0)
              if (fnn->layers[index_layer].elements_depth > 1)
                for (size_t index_neuron = 0; (index_neuron < fnn->layers[index_layer].elements_neurons) && (result == 0); ++index_neuron) {
                  char reentrant_arcs;
                  fscanf(output_stream, "%c ", &reentrant_arcs);
                  if (reentrant_arcs == 'Y') {
                    size_t size = sizeof(double) * fnn->layers[index_layer].elements_neurons;
                    if (((fnn->layers[index_layer].neurons[index_neuron].weights_to_same_layer.values = (double *)d_malloc(size))) &&
                        ((fnn->layers[index_layer].neurons[index_neuron].weights_to_same_layer.corrections = (double *)d_malloc(size))) &&
                        ((fnn->layers[index_layer].neurons[index_neuron].weights_to_same_layer.last_update = (double *)d_malloc(size)))) {
                      for (size_t index_neuron_next_depth = 0; index_neuron_next_depth < fnn->layers[index_layer].elements_neurons; ++index_neuron_next_depth) {
                        fscanf(output_stream, "%lf ", &(fnn->layers[index_layer].neurons[index_neuron].weights_to_same_layer.values[index_neuron_next_depth]));
                        fnn->layers[index_layer].neurons[index_neuron].weights_to_same_layer.last_update[index_neuron_next_depth] = 0;
                      }
                    } else
                      result = 1;
                  }
                }
          }
        }
      }
  } else
    result = 1;
  return result;
}
static void p_fnn_forward_propagation_extended_input(s_fnn *fnn, const size_t input) {
  for (size_t index_input_neurons = 0; index_input_neurons < fnn->layers[0].elements_neurons; ++index_input_neurons)
    fnn->layers[0].neurons[index_input_neurons].values[0].activator = -1.0;
  fnn->layers[0].neurons[input].values[0].activator = 1.0;
  for (size_t index_layer = 1; index_layer < fnn->elements_layers; ++index_layer) {
    for (size_t index_neuron = 0; index_neuron < fnn->layers[index_layer].elements_neurons; ++index_neuron) {
      fnn->layers[index_layer].neurons[index_neuron].values[0].activator_before_integral = fnn->layers[index_layer - 1].bias_to_next_layer[index_neuron];
      for (size_t index_neuron_previous_layer = 0; index_neuron_previous_layer < fnn->layers[index_layer - 1].elements_neurons; ++index_neuron_previous_layer) {
        fnn->layers[index_layer].neurons[index_neuron].values[0].activator_before_integral +=
          (fnn->layers[index_layer - 1].neurons[index_neuron_previous_layer].values[0].activator *
           fnn->layers[index_layer - 1].neurons[index_neuron_previous_layer].weights_to_next_layer.values[index_neuron]);
      }
    }
    /* we need to keep it separated because if the layer uses the softmax function (or another function that depends on the neighborhood, all the
     * "activator_before_integral" should be filled in advance */
    fnn->layers[index_layer].accumulator_exp = 0;
   for (size_t index_neuron = 0; index_neuron < fnn->layers[index_layer].elements_neurons; ++index_neuron)
      fnn->layers[index_layer].accumulator_exp +=
        exp(fnn->layers[index_layer].neurons[index_neuron].values[0].activator_before_integral);
    for (size_t index_neuron = 0; index_neuron < fnn->layers[index_layer].elements_neurons; ++index_neuron)
      fnn->layers[index_layer].neurons[index_neuron].values[0].activator = fnn->layers[index_layer].h_activator_function(&(fnn->layers[index_layer]),
        fnn->layers[index_layer].neurons[index_neuron].values[0].activator_before_integral, false);
  }
}
static void p_fnn_forward_propagation(s_fnn *fnn, const size_t input) {
  if (fnn->elements_layers > 0)
    p_fnn_forward_propagation_extended_input(fnn, input);
}
void f_fnn_run(s_fnn *fnn, const size_t input, size_t *output_index, double *output_score) {
  if (fnn) {
    p_fnn_forward_propagation(fnn, input);
    if ((output_index) && (output_score)) {
      *output_index = 0;
      *output_score = fnn->layers[fnn->elements_layers - 1].neurons[0].values[0].activator;
      for (size_t index = 1; index < fnn->layers[fnn->elements_layers - 1].elements_neurons; ++index)
        if (fnn->layers[fnn->elements_layers - 1].neurons[index].values[0].activator > *output_score) {
          *output_index = index;
          *output_score = fnn->layers[fnn->elements_layers - 1].neurons[index].values[0].activator;
        }
    }
  }
}
static double p_fnn_gradient_clipping(double gradient, double clipping_gradient_value) {
  if (gradient > clipping_gradient_value)
    gradient = clipping_gradient_value;
  else if (gradient < -(clipping_gradient_value))
    gradient = -(clipping_gradient_value);
  return gradient;
}
void p_fnn_back_propagation_calculate_bias_error_responsibility_to_next_layer_and_arc_corrections(s_neural_layer *layer,
  s_neural_layer *next_layer_with_gradients, double learning_rate, size_t index_depth) {
  for (size_t index_next_layer_neuron = 0; index_next_layer_neuron < next_layer_with_gradients->elements_neurons; ++index_next_layer_neuron) {
    if (index_depth == 0)
      layer->bias_corrections[index_next_layer_neuron] = 0;
    layer->bias_corrections[index_next_layer_neuron] += next_layer_with_gradients->neurons[index_next_layer_neuron].values[index_depth].local_gradient;
  }
  for (size_t index_neuron = 0; index_neuron < layer->elements_neurons; ++index_neuron) {
    layer->neurons[index_neuron].values[index_depth].error_responsibility_to_next_layer = 0;
    for (size_t index_next_layer_neuron = 0; index_next_layer_neuron < next_layer_with_gradients->elements_neurons; ++index_next_layer_neuron)
      layer->neurons[index_neuron].values[index_depth].error_responsibility_to_next_layer +=
        (next_layer_with_gradients->neurons[index_next_layer_neuron].values[index_depth].local_gradient *
         layer->neurons[index_neuron].weights_to_next_layer.values[index_next_layer_neuron]);
    if ((layer->neurons[index_neuron].weights_to_same_layer.values) && (index_depth < (layer->elements_depth - 1)))
      for (size_t index_same_layer_neuron = 0; index_same_layer_neuron < layer->elements_neurons; ++index_same_layer_neuron)
        layer->neurons[index_neuron].values[index_depth].error_responsibility_to_next_layer +=
          (layer->neurons[index_same_layer_neuron].values[index_depth + 1].local_gradient *
          layer->neurons[index_neuron].weights_to_same_layer.values[index_same_layer_neuron]);
  }
}
void p_fnn_back_propagation_compute_local_gradient(s_neural_layer *layer_with_error_responsibility, double clipping_gradient_value, size_t index_depth) {
  layer_with_error_responsibility->accumulator_exp = 0;
  for (size_t index_neuron = 0; index_neuron < layer_with_error_responsibility->elements_neurons; ++index_neuron)
    layer_with_error_responsibility->accumulator_exp += exp(
      layer_with_error_responsibility->neurons[index_neuron].values[index_depth].activator_before_integral);
  for (size_t index_neuron = 0; index_neuron < layer_with_error_responsibility->elements_neurons; ++index_neuron)
    layer_with_error_responsibility->neurons[index_neuron].values[index_depth].local_gradient = p_fnn_gradient_clipping(
      layer_with_error_responsibility->neurons[index_neuron].values[index_depth].error_responsibility_to_next_layer *
      layer_with_error_responsibility->h_activator_function(layer_with_error_responsibility,
        layer_with_error_responsibility->neurons[index_neuron].values[index_depth].activator, true), clipping_gradient_value);
}
void p_fnn_back_propagation_update_arc_weights(s_neural_layer *layer_with_arc_corrections, s_neural_layer *next_layer_with_gradients,
  size_t elements_next_layer, double momentum_gradient, double learning_rate, size_t time_frames) {
  for (size_t index_depth = 0; index_depth < time_frames; ++index_depth) {
    for (size_t index_neuron = 0; index_neuron < layer_with_arc_corrections->elements_neurons; ++index_neuron) {
      if (next_layer_with_gradients)
        for (size_t index_neuron_next_layer = 0; index_neuron_next_layer < next_layer_with_gradients->elements_neurons; ++index_neuron_next_layer) {
          if (!index_depth)
            layer_with_arc_corrections->neurons[index_neuron].weights_to_next_layer.corrections[index_neuron_next_layer] = 0;
          layer_with_arc_corrections->neurons[index_neuron].weights_to_next_layer.corrections[index_neuron_next_layer] +=
            (next_layer_with_gradients->neurons[index_neuron_next_layer].values[index_depth].local_gradient *
             layer_with_arc_corrections->neurons[index_neuron].values[index_depth].activator);
        }
      if ((layer_with_arc_corrections->neurons[index_neuron].weights_to_same_layer.values) && (index_depth < (time_frames - 1)))
        for (size_t index_neuron_same_layer = 0; index_neuron_same_layer < layer_with_arc_corrections->elements_neurons; ++index_neuron_same_layer) {
          if (!index_depth)
            layer_with_arc_corrections->neurons[index_neuron].weights_to_same_layer.corrections[index_neuron_same_layer] = 0;
          layer_with_arc_corrections->neurons[index_neuron].weights_to_same_layer.corrections[index_neuron_same_layer] +=
            (layer_with_arc_corrections->neurons[index_neuron_same_layer].values[index_depth + 1].local_gradient *
             layer_with_arc_corrections->neurons[index_neuron].values[index_depth].activator);
        }
    }
  }
  for (size_t index_neuron = 0; index_neuron < layer_with_arc_corrections->elements_neurons; ++index_neuron) {
    if (next_layer_with_gradients)
      for (size_t index_neuron_next_layer = 0; index_neuron_next_layer < elements_next_layer; ++index_neuron_next_layer) {
        double correction_clipped = d_fnn_clip(layer_with_arc_corrections->neurons[index_neuron].weights_to_next_layer.corrections[index_neuron_next_layer],
          d_fnn_clipping_correction), new_update = (learning_rate * correction_clipped) +
              (momentum_gradient * layer_with_arc_corrections->neurons[index_neuron].weights_to_next_layer.last_update[index_neuron_next_layer]);
        layer_with_arc_corrections->neurons[index_neuron].weights_to_next_layer.values[index_neuron_next_layer] -= new_update;
        layer_with_arc_corrections->neurons[index_neuron].weights_to_next_layer.last_update[index_neuron_next_layer] = new_update;
      }
    if (layer_with_arc_corrections->neurons[index_neuron].weights_to_same_layer.values)
      for (size_t index_same_layer_neuron = 0; index_same_layer_neuron < layer_with_arc_corrections->elements_neurons; ++index_same_layer_neuron) {
        double correction_clipped = d_fnn_clip(layer_with_arc_corrections->neurons[index_neuron].weights_to_same_layer.corrections[index_same_layer_neuron],
          d_fnn_clipping_correction), new_update = (learning_rate * correction_clipped) +
            (momentum_gradient * layer_with_arc_corrections->neurons[index_neuron].weights_to_same_layer.last_update[index_same_layer_neuron]);
        layer_with_arc_corrections->neurons[index_neuron].weights_to_same_layer.values[index_same_layer_neuron] -= new_update;
        layer_with_arc_corrections->neurons[index_neuron].weights_to_same_layer.last_update[index_same_layer_neuron] = new_update;
      }
  }
  if (next_layer_with_gradients)
    if ((layer_with_arc_corrections->bias_to_next_layer) && (layer_with_arc_corrections->bias_corrections))
      for (size_t index_bias = 0; index_bias < next_layer_with_gradients->elements_neurons; ++index_bias) {
        double bias_correction_clipped = d_fnn_clip(layer_with_arc_corrections->bias_corrections[index_bias], d_fnn_clipping_correction),
        new_update = (learning_rate * bias_correction_clipped) + (momentum_gradient * layer_with_arc_corrections->bias_last_update[index_bias]);
        layer_with_arc_corrections->bias_to_next_layer[index_bias] -= new_update;
        layer_with_arc_corrections->bias_last_update[index_bias] = new_update;
      }
}
static void p_fnn_back_propagation(s_fnn *fnn, const size_t target_output) {
  if (fnn->elements_layers > 0) {
    fnn->recent_average_error = 0;
    for (size_t index_neuron = 0; index_neuron < fnn->layers[fnn->elements_layers - 1].elements_neurons; ++index_neuron) {
      double residual_neuron = (-1.0 - fnn->layers[fnn->elements_layers - 1].neurons[index_neuron].values[0].activator);
      if (target_output == index_neuron)
        residual_neuron = (1.0 - fnn->layers[fnn->elements_layers - 1].neurons[index_neuron].values[0].activator);
      fnn->recent_average_error += (residual_neuron * residual_neuron);
      fnn->layers[fnn->elements_layers - 1].neurons[index_neuron].values[0].error_responsibility_to_next_layer = residual_neuron;
    }
    p_fnn_back_propagation_compute_local_gradient(&(fnn->layers[fnn->elements_layers - 1]), fnn->clipping_gradient, 0);
    fnn->recent_average_error = (fnn->recent_average_error / 2.0);
    for (int index_layer = (int)(fnn->elements_layers - 2); index_layer >= 0; --index_layer) {
      p_fnn_back_propagation_calculate_bias_error_responsibility_to_next_layer_and_arc_corrections(&(fnn->layers[index_layer]),
        &(fnn->layers[index_layer + 1]), fnn->learning_rate, 0);
      p_fnn_back_propagation_compute_local_gradient(&(fnn->layers[index_layer]), fnn->clipping_gradient, 0);
    }
    for (int index_layer = (int)(fnn->elements_layers - 2); index_layer >= 0; --index_layer)
      p_fnn_back_propagation_update_arc_weights(&(fnn->layers[index_layer]), &(fnn->layers[index_layer + 1]), fnn->layers[index_layer + 1].elements_neurons,
        fnn->momentum_gradient, fnn->learning_rate, 1);
    ++fnn->learning_epoch;
  }
}
void f_fnn_train(s_fnn *fnn, const size_t input, const size_t target_output, double *optional_output_sequence) {
  if (fnn) {
    p_fnn_forward_propagation(fnn, input);
    p_fnn_back_propagation(fnn, target_output);
    if (optional_output_sequence)
      for (size_t index_neuron = 0; index_neuron < fnn->layers[fnn->elements_layers - 1].elements_neurons; ++index_neuron)
        optional_output_sequence[index_neuron] = fnn->layers[fnn->elements_layers - 1].neurons[index_neuron].values[0].activator;
  }
}
void f_fnn_free(s_fnn *fnn) {
  if (fnn) {
    for (size_t index_layer = 0; index_layer < fnn->elements_layers; ++index_layer) {
      if (fnn->layers[index_layer].bias_to_next_layer)
        d_free(fnn->layers[index_layer].bias_to_next_layer);
      if (fnn->layers[index_layer].bias_corrections)
        d_free(fnn->layers[index_layer].bias_corrections);
      if (fnn->layers[index_layer].bias_last_update)
        d_free(fnn->layers[index_layer].bias_last_update);
      for (size_t index_neuron = 0; index_neuron < fnn->layers[index_layer].elements_neurons; ++index_neuron) {
        d_free(fnn->layers[index_layer].neurons[index_neuron].values);
        if (fnn->layers[index_layer].neurons[index_neuron].weights_to_next_layer.values) {
          d_free(fnn->layers[index_layer].neurons[index_neuron].weights_to_next_layer.values);
          d_free(fnn->layers[index_layer].neurons[index_neuron].weights_to_next_layer.corrections);
          d_free(fnn->layers[index_layer].neurons[index_neuron].weights_to_next_layer.last_update);
        }
        if (fnn->layers[index_layer].neurons[index_neuron].weights_to_same_layer.values) {
          d_free(fnn->layers[index_layer].neurons[index_neuron].weights_to_same_layer.values);
          d_free(fnn->layers[index_layer].neurons[index_neuron].weights_to_same_layer.corrections);
          d_free(fnn->layers[index_layer].neurons[index_neuron].weights_to_same_layer.last_update);
        }
      }
      d_free(fnn->layers[index_layer].neurons);
    }
    d_free(fnn->layers);
  }
}