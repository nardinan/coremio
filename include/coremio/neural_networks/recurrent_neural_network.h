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
#ifndef COREMIO_RECURRENT_NEURAL_NETWORK_H
#define COREMIO_RECURRENT_NEURAL_NETWORK_H
#include "feedforward_neural_network.h"
#define d_rnn_smooth_loss_alpha 0.01
typedef struct s_rnn {
  size_t time_frames;
  s_fnn core;
  /* convergence metrics */
  double smooth_loss, perplexity, accuracy;
  size_t correct_predictions, total_predictions;
} s_rnn;
extern int f_rnn_new(s_rnn *rnn, double learning_rate, double momentum_gradient, size_t time_frames, size_t layers, ...);
extern void f_rnn_dump_model(s_rnn *rnn, FILE *output_stream);
extern int f_rnn_load_model(s_rnn *rnn, FILE *output_stream);
extern void f_rnn_run(s_rnn *rnn, const size_t *input, size_t *output_index, double *output_score);
extern void f_rnn_train(s_rnn *rnn, const size_t *input, const size_t *target_output, double *optional_output_sequence);
extern size_t f_rnn_generate_step(s_rnn *rnn, size_t input, double *output_probabilities, double temperature);
extern void f_rnn_reset_state(s_rnn *rnn);
extern void f_rnn_free(s_rnn *rnn);
#endif //COREMIO_RECURRENT_NEURAL_NETWORK_H