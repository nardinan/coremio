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
#ifndef CURL_H
#define CURL_H
#define d_destination_length 2048
#include <curl/curl.h>
#include "dictionary.h"
#include "list.h"
#include "result.h"
#include "runner.h"
d_result_declare(SHIT_CURL_HEADERS_PACK);
typedef enum e_curl_callback_methoads {
  e_curl_callback_method_get,
  e_curl_callback_method_post,
  e_curl_callback_method_put,
  e_curl_callback_method_patch,
  e_curl_callback_method_delete
} e_curl_callback_methods;
typedef struct s_curl_callback_runner_header_entry {
  s_dictionary_node head;
  char *value;
  bool free_value;
} s_curl_callback_runner_header_entry;
typedef struct s_curl_callback_runner_payload {
  s_dictionary header;
  char *payload;
  size_t payload_container_size, payload_content_size;
} s_curl_callback_runner_payload;
typedef struct s_curl_callback_runner {
  s_runner head;
  char destination[d_destination_length];
  e_curl_callback_methods method;
  s_curl_callback_runner_payload send, received;
} s_curl_callback_runner;
typedef struct s_curl_callback {
  s_list_node head;
  unsigned int id;
  s_curl_callback_runner runner;
} s_curl_callback;
extern void f_curl_callback_initialize(s_curl_callback *curl, unsigned int id, const char *destination, e_curl_callback_methods method);
extern coremio_result f_curl_callback_add_send_header_entry(s_curl_callback *curl, const char *header_key, char *header_value, const bool free_value);
extern void f_curl_callback_run(s_curl_callback *curl, const char *payload);
#endif // CURL_H
