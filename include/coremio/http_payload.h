/**
 * MIT License
 * Copyright (c) [2026] The Barfing Fox - TBF [nardinan (andrea@nardinan.it)]
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
#ifndef HTTP_PAYLOAD_H
#define HTTP_PAYLOAD_H
#include <ctype.h>
#include "dictionary.h"
#include "result.h"
#define d_http_sequence_new_line_characters "\r\n"
#define d_http_sequence_skippable_prefix_characters " \t\r\n"
d_result_declare(SHIT_HTTP_PAYLOAD_INCOMPLETE);
typedef enum e_http_sequence_steps {
  e_http_sequence_step_A_header_block = 0,
  e_http_sequence_step_B_header_block,
  e_http_sequence_step_C_header_block,
  e_http_sequence_step_key_value,
  e_http_sequence_step_key_value_again,
  e_http_sequence_step_payload,
  e_http_sequence_step_completed
} e_http_sequence_steps;
typedef enum e_http_methods {
  e_http_method_undefined = 0,
  e_http_method_get,
  e_http_method_head,
  e_http_method_options,
  e_http_method_trace,
  e_http_method_put,
  e_http_method_delete,
  e_http_method_post,
  e_http_method_patch,
  e_http_method_connect,
} e_http_methods;
extern const char *m_http_methods_keywords[];
typedef struct s_http_payload_value_node {
  s_dictionary_node head;
  char *value, *raw_payload_key_value;
} s_http_payload_value_node;
typedef struct s_http_payload {
  e_http_sequence_steps current_sequence_step;
  e_http_methods enumerated_method;
  char *method, *path, *version, *status_message;
  unsigned int status_code;
  s_dictionary configuration;
  char *body;
} s_http_payload;
#define d_http_payload_buffer_size 256
#define d_http_payload_buffer_minimum_space_before_increment 32
extern coremio_result f_http_payload_read(int descriptor, s_http_payload *http_payload, unsigned char **in_buffer, size_t *buffer_size, size_t *payload_size,
    size_t *shift_unserialized_payload_size, time_t timeout_milliseconds);
extern void f_http_payload_initialize(s_http_payload *http_payload);
extern coremio_result f_http_payload_unserialize(s_http_payload *http_payload, char *raw_payload, const size_t buffer_size, size_t *shift_unserialized_size);
extern coremio_result f_http_payload_serialize(s_http_payload *http_payload, unsigned char **raw_payload, size_t *buffer_size);
extern void f_http_payload_free(s_http_payload *http_payload);
#endif // HTTP_PAYLOAD_H
