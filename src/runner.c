/**
 * MIT License
 * Copyright (c) [2026] The Barfing Fox [Andrea Nardinocchi (andrea@nardinan.it)]
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
#include "../include/coremio/runner.h"
d_result_define(SHIT_THREAD_FAILURE, 1, "Failure, it seems impossible to correctly launch the thread");
d_result_define(SHIT_THREAD_INTERRUPTED, 2, "Running thread has been interrupted");
coremio_result f_runner_initialize(s_runner *runner, l_runner_user_callback user_callback) {
  coremio_result result = SHIT_NO_MEMORY;
  memset(runner, 0, sizeof(s_runner));
  if (pthread_mutex_init(&(runner->status_lock), NULL) == 0) {
    if (pthread_mutex_init(&(runner->result_lock), NULL) != 0)
      pthread_mutex_destroy(&(runner->status_lock));
    else if (pthread_mutex_init(&(runner->interrupt_lock), NULL) != 0) {
      pthread_mutex_destroy(&(runner->status_lock));
      pthread_mutex_destroy(&(runner->result_lock));
    } else if (pthread_mutex_init(&(runner->join_lock), NULL) != 0) {
      pthread_mutex_destroy(&(runner->status_lock));
      pthread_mutex_destroy(&(runner->result_lock));
      pthread_mutex_destroy(&(runner->interrupt_lock));
    } else if (pthread_cond_init(&(runner->complete_trigger), NULL) != 0) {
      pthread_mutex_destroy(&(runner->status_lock));
      pthread_mutex_destroy(&(runner->result_lock));
      pthread_mutex_destroy(&(runner->interrupt_lock));
      pthread_mutex_destroy(&(runner->join_lock));
    } else
      result = NOICE;
  }
  if (result == NOICE) {
    runner->user_callback = user_callback;
    runner->status = e_runner_status_idle;
  }
  return result;
}
e_runner_statuses f_runner_get_status(s_runner *runner) {
  e_runner_statuses status;
  d_runner_safely_access_resource(runner->status_lock, status, runner->status);
  return status;
}
coremio_result f_runner_get_result(s_runner *runner) {
  coremio_result result = SHIT_NOT_INITIALIZED;
  if (f_runner_get_status(runner) == e_runner_status_completed)
    d_runner_safely_access_resource(runner->result_lock, result, runner->result);
  return result;
}
static void *p_runner_run_callback(s_runner *runner) {
  coremio_result result = NOICE;
  if (f_runner_get_status(runner) == e_runner_status_launching) {
    e_runner_statuses final_status;
    d_runner_safely_access_resource(runner->status_lock, runner->status, e_runner_status_running);
    if (runner->user_callback)
      result = runner->user_callback(runner, runner->user_data);
    if (result == SHIT_THREAD_INTERRUPTED)
      final_status = e_runner_status_interrupted;
    else
      final_status = e_runner_status_completed;
    d_runner_safely_access_resource(runner->result_lock, runner->result, result);
    d_runner_safely_access_resource(runner->status_lock, runner->status, final_status);
  }
  pthread_cond_broadcast(&(runner->complete_trigger));
  return NULL;
}
coremio_result f_runner_run(s_runner *runner, void *user_data) {
  coremio_result result = SHIT_ALREADY_INITIALIZED;
  e_runner_statuses current_status = f_runner_get_status(runner);
  if ((current_status == e_runner_status_completed) || (current_status == e_runner_status_interrupted)) {
    f_runner_join(runner);
  }
  pthread_mutex_lock(&(runner->status_lock));
  {
    if ((current_status = runner->status) == e_runner_status_idle) {
      runner->status = e_runner_status_launching;
    }
  }
  pthread_mutex_unlock(&(runner->status_lock));
  if (current_status == e_runner_status_idle) {
    runner->user_data = user_data;
    d_runner_safely_access_resource(runner->interrupt_lock, runner->interrupt_required, false);
    if (pthread_create(&(runner->internal_callback_thread), NULL, (void *(*)(void *) ) p_runner_run_callback, (void *) runner) != 0) {
      d_runner_safely_access_resource(runner->status_lock, runner->status, e_runner_status_idle);
      result = SHIT_THREAD_FAILURE;
    } else
      result = NOICE;
  }
  return result;
}
bool f_runner_is_running(s_runner *runner) {
  e_runner_statuses status = f_runner_get_status(runner);
  return ((status == e_runner_status_launching) || (status == e_runner_status_running));
}
void f_runner_join(s_runner *runner) {
  e_runner_statuses current_status;
  pthread_mutex_lock(&(runner->join_lock));
  {
    d_runner_safely_access_resource(runner->status_lock, current_status, runner->status);
    if ((current_status == e_runner_status_launching) || (current_status == e_runner_status_running) || (current_status == e_runner_status_completed) ||
        (current_status == e_runner_status_interrupted)) {
      pthread_join(runner->internal_callback_thread, NULL);
      d_runner_safely_access_resource(runner->status_lock, runner->status, e_runner_status_idle);
    }
  }
  pthread_mutex_unlock(&(runner->join_lock));
}
coremio_result f_runner_tryjoin(s_runner *runner, time_t milliseconds) {
  coremio_result result = SHIT_TIMEOUT;
  struct timespec timeout;
  bool timeout_reached = false;
  clock_gettime(CLOCK_REALTIME, &timeout);
  timeout.tv_sec += (milliseconds / 1000);
  if ((timeout.tv_nsec += ((milliseconds % 1000) * 1000000)) >= 1000000000) {
    timeout.tv_nsec -= 1000000000;
    ++timeout.tv_sec;
  }
  pthread_mutex_lock(&(runner->status_lock));
  {
    while ((!timeout_reached) && ((runner->status == e_runner_status_running) || (runner->status == e_runner_status_launching)))
      if (pthread_cond_timedwait(&(runner->complete_trigger), &(runner->status_lock), &timeout) == ETIMEDOUT)
        timeout_reached = true;
  }
  pthread_mutex_unlock(&(runner->status_lock));
  if (!timeout_reached) {
    f_runner_join(runner);
    result = NOICE;
  }
  return result;
}
coremio_result f_runner_stop(s_runner *runner) {
  coremio_result result = SHIT_NOT_INITIALIZED;
  if (f_runner_get_status(runner) == e_runner_status_running) {
    d_runner_safely_access_resource(runner->interrupt_lock, runner->interrupt_required, true);
    result = NOICE;
  }
  return result;
}
void f_runner_free(s_runner *runner, bool force_stop) {
  if (force_stop)
    f_runner_stop(runner);
  f_runner_join(runner);
  d_runner_safely_access_resource(runner->status_lock, runner->status, e_runner_status_undefined);
  pthread_mutex_destroy(&(runner->status_lock));
  pthread_mutex_destroy(&(runner->result_lock));
  pthread_mutex_destroy(&(runner->interrupt_lock));
  pthread_mutex_destroy(&(runner->join_lock));
  pthread_cond_destroy(&(runner->complete_trigger));
}
