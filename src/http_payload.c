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
#include "../include/coremio/http_payload.h"
d_result_define(SHIT_HTTP_PAYLOAD_INCOMPLETE, 1, "Failure: impossible to unserialize the payload as it seems to be incomplete");
static void p_http_payload_value_node_free(s_http_payload_value_node *http_payload_node) {
  if (http_payload_node->raw_payload_key_value) {
    d_free(http_payload_node->raw_payload_key_value);
    http_payload_node->raw_payload_key_value = NULL;
  }
}
void f_http_payload_initialize(s_http_payload *http_payload) {
  memset(http_payload, 0, sizeof(s_http_payload));
  f_dictionary_initialize_custom(&(http_payload->configuration), sizeof(s_http_payload_value_node), NULL,
      (l_dictionary_node_delete) p_http_payload_value_node_free);
}
static char *p_http_payload_unserialize_block(char *raw_payload, const char *needle, const size_t total_size, size_t *shift_unserialized_size,
    ssize_t *current_session_shift_size) {
  char *result = NULL;
  if (current_session_shift_size)
    *current_session_shift_size = -1;
  if (total_size > *shift_unserialized_size) {
    char *raw_payload_active = (raw_payload + *shift_unserialized_size), *raw_payload_terminal = NULL;
    if ((raw_payload_terminal = strstr(raw_payload_active, needle))) {
      const size_t length_needle = strlen(needle);
      size_t length_block = 0;
      if (current_session_shift_size)
        *current_session_shift_size = 0;
      if ((raw_payload_terminal) && ((length_block = (raw_payload_terminal - raw_payload_active)) > 0)) {
        /* token is complete, let's analyze it */
        if ((result = (char *) d_malloc(length_block + 1))) {
          strncpy(result, raw_payload_active, length_block);
          result[length_block] = 0;
          *shift_unserialized_size += length_block;
          if (current_session_shift_size)
            *current_session_shift_size += length_block;
        }
      }
      *shift_unserialized_size += length_needle;
      if (current_session_shift_size)
        *current_session_shift_size += length_needle;
    }
  }
  return result;
}
coremio_result f_http_payload_unserialize(s_http_payload *http_payload, char *raw_payload, const size_t total_size, size_t *shift_unserialized_size) {
  coremio_result result = NOICE;
  if ((http_payload->current_sequence_step < e_http_sequence_step_completed) && (total_size > *shift_unserialized_size)) {
    char *raw_payload_active, *raw_payload_key_value;
    ssize_t current_session_shift_size;
    e_http_sequence_steps previous_sequence_step;
    do {
      previous_sequence_step = http_payload->current_sequence_step;
      switch (http_payload->current_sequence_step) {
        case e_http_sequence_step_type: {
          if ((http_payload->request_type = p_http_payload_unserialize_block(raw_payload, " ", total_size, shift_unserialized_size, NULL)))
            http_payload->current_sequence_step = e_http_sequence_step_parameters;
          break;
        }
        case e_http_sequence_step_parameters: {
          if ((http_payload->request_parameters = p_http_payload_unserialize_block(raw_payload, " ", total_size, shift_unserialized_size, NULL)))
            http_payload->current_sequence_step = e_http_sequence_step_version;
          break;
        }
        case e_http_sequence_step_version: {
          if ((http_payload->version = p_http_payload_unserialize_block(raw_payload, d_http_sequence_new_line_characters, total_size, shift_unserialized_size,
                   NULL)))
            http_payload->current_sequence_step = e_http_sequence_step_key_value;
          break;
        }
        case e_http_sequence_step_key_value: {
          if (((raw_payload_key_value = p_http_payload_unserialize_block(raw_payload, d_http_sequence_new_line_characters, total_size, shift_unserialized_size,
                    &current_session_shift_size)) == NULL) &&
              (current_session_shift_size >= 0)) {
            http_payload->current_sequence_step = e_http_sequence_step_payload;
          } else if (raw_payload_key_value) {
            char *raw_payload_value = strchr(raw_payload_key_value, ':');
            if (raw_payload_value) {
              size_t key_length, value_length;
              *raw_payload_value = 0;
              ++raw_payload_value;
              while ((*raw_payload_value) && (strchr(d_http_sequence_skippable_prefix_characters, *raw_payload_value)))
                ++raw_payload_value;
              if (((key_length = strlen(raw_payload_key_value)) > 0) && ((value_length = strlen(raw_payload_value)) > 0)) {
                while ((value_length > 0) && (strchr(d_http_sequence_skippable_prefix_characters, raw_payload_value[value_length - 1]))) {
                  raw_payload_value[value_length - 1] = 0;
                  --value_length;
                }
                if (value_length > 0) {
                  s_http_payload_value_node *payload_value_node;
                  /* before moving ahead, I'll need to convert lowercase data */
                  for (size_t index_character = 0; index_character < key_length; ++index_character)
                    raw_payload_key_value[index_character] = tolower(raw_payload_key_value[index_character]);
                  if ((payload_value_node = (s_http_payload_value_node *) f_dictionary_get_or_create(&(http_payload->configuration), raw_payload_key_value))) {
                    if (payload_value_node->raw_payload_key_value)
                      d_free(payload_value_node->raw_payload_key_value);
                    payload_value_node->raw_payload_key_value = raw_payload_key_value;
                    payload_value_node->value = raw_payload_value;
                    raw_payload_key_value = NULL; /* we hooked the allocated string to the node */
                    http_payload->current_sequence_step = e_http_sequence_step_key_value_again;
                  }
                }
              }
            }
            if (raw_payload_key_value)
              d_free(raw_payload_key_value);
          }
          break;
        }
        case e_http_sequence_step_key_value_again: {
          http_payload->current_sequence_step = e_http_sequence_step_key_value;
          break;
        }
        case e_http_sequence_step_payload: {
          s_http_payload_value_node *payload_value_node;
          ssize_t payload_length = 0;
          if (((payload_value_node = (s_http_payload_value_node *) f_dictionary_get_if_exists(&(http_payload->configuration), "content-length"))) &&
              ((payload_value_node->value)))
            payload_length = atoi(payload_value_node->value);
          if (payload_length > 0) {
            if ((total_size > *shift_unserialized_size) && ((total_size - *shift_unserialized_size) >= payload_length) &&
                ((raw_payload_active = raw_payload + *shift_unserialized_size))) {
              if ((http_payload->body = (char *) d_malloc(payload_length + 1))) {
                strncpy(http_payload->body, raw_payload_active, payload_length);
                http_payload->body[payload_length] = 0;
                (*shift_unserialized_size) += payload_length;
                http_payload->current_sequence_step = e_http_sequence_step_completed;
              }
            }
          } else
            http_payload->current_sequence_step = e_http_sequence_step_completed;
          break;
        }
        case e_http_sequence_step_completed:
        default: {
          break;
        }
      }
    } while ((http_payload->current_sequence_step != previous_sequence_step) && (http_payload->current_sequence_step != e_http_sequence_step_completed));
  }
  if ((http_payload->current_sequence_step != e_http_sequence_step_completed) && (result == NOICE))
    result = SHIT_HTTP_PAYLOAD_INCOMPLETE;
  return result;
}
void f_http_payload_free(s_http_payload *http_payload) {
  http_payload->current_sequence_step = e_http_sequence_step_type;
  if (http_payload->request_type)
    d_free(http_payload->request_type);
  if (http_payload->request_parameters)
    d_free(http_payload->request_parameters);
  if (http_payload->version)
    d_free(http_payload->version);
  if (http_payload->body)
    d_free(http_payload->body);
  f_dictionary_free(&(http_payload->configuration));
}
