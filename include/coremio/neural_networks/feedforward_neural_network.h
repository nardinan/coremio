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
#ifndef COREMIO_FEEDFORWARD_NEURAL_NETWORK_H
#define COREMIO_FEEDFORWARD_NEURAL_NETWORK_H
#include "../json.h"
#define d_fnn_clipping_gradient 0.85
#define d_fnn_clipping_correction 5.0
#define d_fnn_clip(value,clip) (((value)>(clip))?(clip):(((value)<(-(clip)))?(-(clip)):(value)))
struct s_neural_layer;
typedef double (*t_fnn_transfer_function)(struct s_neural_layer*, double, bool);
typedef struct s_link_weight {
  double *values, *corrections, *last_update;
} s_link_weight;
typedef struct s_neuron_values {
  double local_gradient, activator_before_integral, activator, error_responsibility_to_next_layer;
} s_neuron_values;
typedef struct s_neuron {
  s_neuron_values* values;
  s_link_weight weights_to_next_layer, weights_to_same_layer;
} s_neuron;
typedef struct s_neural_layer {
  s_neuron* neurons;
  double *bias_to_next_layer, *bias_corrections, *bias_last_update, accumulator_exp;
  t_fnn_transfer_function h_activator_function;
  size_t elements_neurons, elements_depth;
} s_neural_layer;
typedef struct s_fnn {
  s_neural_layer* layers;
  double learning_rate, momentum_gradient, recent_average_error, clipping_gradient;
  size_t elements_layers, learning_epoch;
} s_fnn;
extern double p_fnn_hyperbolic_tangent_function(s_neural_layer* layer, double value, bool derivative);
extern double p_fnn_sigmoid_function(s_neural_layer* layer, double value, bool derivative);
extern double p_fnn_rectifier_function(s_neural_layer* layer, double value, bool derivative);
extern double p_fnn_softmax_function(s_neural_layer* layer, double value, bool derivative);
extern double p_fnn_random_weight(const double elements_in_layer, const bool same_layer);
extern int p_fnn_layer_new(s_neural_layer* layer, const size_t elements_neurons, const size_t elements_depth, const double dropout_rate,
  const bool reentrant_arcs_needed, s_neural_layer* previous_layer);
extern int p_fnn_container_new(s_fnn* fnn, const double learning_rate, const double momentum_gradient, const size_t layers);
extern int f_fnn_new(s_fnn* fnn, double learning_rate, double momentum_gradient, size_t layers, ...);
extern void f_fnn_dump_model(s_fnn* fnn, FILE* output_stream);
extern int f_fnn_load_model(s_fnn* fnn, FILE* output_stream);
extern void f_fnn_run(s_fnn* fnn, size_t input, size_t* output_index, double* output_score);
extern void p_fnn_back_propagation_calculate_bias_error_responsibility_to_next_layer_and_arc_corrections(s_neural_layer* layer,
  s_neural_layer* next_layer_with_gradients, double learning_rate, size_t index_depth);
extern void p_fnn_back_propagation_compute_local_gradient(s_neural_layer* layer_with_error_responsibility, double clipping_gradient_value, size_t index_depth);
extern void p_fnn_back_propagation_update_arc_weights(s_neural_layer* layer_with_arc_corrections, s_neural_layer* next_layer_with_gradients,
  size_t elements_next_layer, double momentum_gradient, double learning_rate, size_t time_frames);
extern void f_fnn_train(s_fnn* fnn, const size_t input, const size_t target_output, double* optional_output_sequence);
extern void f_fnn_free(s_fnn* fnn);
#endif //COREMIO_FEEDFORWARD_NEURAL_NETWORK_H
