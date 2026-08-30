/**
 * MIT License
 * Copyright (c) [2026] The Barfing Fox [Andrea Nardinocchi
 * (andrea@nardinan.it)]
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.§
 */
#ifndef COREMIO_RUNNER_H
#define COREMIO_RUNNER_H
#include <errno.h>
#include <pthread.h>
#include <stdbool.h>
#include <string.h>
#include "result.h"
d_result_declare(SHIT_THREAD_FAILURE);
d_result_declare(SHIT_THREAD_INTERRUPTED);
typedef enum e_runner_statuses {
  e_runner_status_idle = 0,
  e_runner_status_launching,
  e_runner_status_running,
  e_runner_status_completed,
  e_runner_status_interrupted,
  e_runner_status_undefined
} e_runner_statuses;
struct s_runner;
typedef coremio_result (*l_runner_user_callback)(struct s_runner *self, void *user_data);
typedef struct s_runner {
  bool interrupt_required;
  pthread_t internal_callback_thread;
  l_runner_user_callback user_callback;
  void *user_data;
  pthread_mutex_t status_lock, result_lock, interrupt_lock, join_lock;
  pthread_cond_t complete_trigger;
  e_runner_statuses status;
  coremio_result result;
} s_runner;
#define d_runner_safely_access_resource(l, s, d)                                                                                                               \
  do {                                                                                                                                                         \
    pthread_mutex_lock(&(l));                                                                                                                                  \
    (s) = (d);                                                                                                                                                 \
    pthread_mutex_unlock(&(l));                                                                                                                                \
  } while (0)
#define d_runner_interruptable_point(r)                                                                                                                        \
  do {                                                                                                                                                         \
    bool interrupt_required = false;                                                                                                                           \
    if (pthread_mutex_trylock(&((r)->interrupt_lock)) == 0) {                                                                                                  \
      interrupt_required = (r)->interrupt_required;                                                                                                            \
      pthread_mutex_unlock(&((r)->interrupt_lock));                                                                                                            \
    }                                                                                                                                                          \
    if (interrupt_required)                                                                                                                                    \
      return SHIT_THREAD_INTERRUPTED;                                                                                                                          \
  } while (0)
extern coremio_result f_runner_initialize(s_runner *runner, l_runner_user_callback user_callback);
extern e_runner_statuses f_runner_get_status(s_runner *runner);
extern coremio_result f_runner_get_result(s_runner *runner);
extern coremio_result f_runner_run(s_runner *runner, void *user_data);
extern bool f_runner_is_running(s_runner *runner);
extern void f_runner_join(s_runner *runner);
extern coremio_result f_runner_tryjoin(s_runner *runner, time_t milliseconds);
extern coremio_result f_runner_stop(s_runner *runner);
extern void f_runner_free(s_runner *runner, bool force_stop);
#endif // COREMIO_RUNNER_H
