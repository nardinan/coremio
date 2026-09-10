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
#include "../include/coremio/server.h"
d_result_define(SHIT_HTTP_PAYLOAD_INCOMPLETE, 1, "Failure: impossible to unserialize the payload as it seems to be incomplete");
const char *m_http_methods_keywords[] = {"GET", "HEAD", "OPTIONS", "TRACE", "PUT", "DELETE", "POST", "PATCH", "CONNECT", NULL};
static e_http_methods p_http_method_from_keyword(const char *keyword) {
  e_http_methods result = e_http_method_undefined;
  for (size_t index_keyword = 0; ((result == e_http_method_undefined) && (m_http_methods_keywords[index_keyword])); ++index_keyword)
    if (strcmp(m_http_methods_keywords[index_keyword], keyword) == 0)
      result = (e_http_methods) (index_keyword + 1);
  return result;
}
coremio_result f_http_payload_read(int descriptor, s_http_payload *http_payload, unsigned char **in_buffer, size_t *buffer_size, size_t *payload_size,
    size_t *shift_unserialized_payload_size, time_t timeout_milliseconds) {
  coremio_result result = NOICE;
  size_t residual_space = 0;
  do {
    if (*buffer_size >= *payload_size) {
      if ((!(*in_buffer)) || ((residual_space = (*buffer_size - *payload_size)) < d_http_payload_buffer_minimum_space_before_increment)) {
        unsigned char *new_in_buffer;
        if ((new_in_buffer = (unsigned char *) d_realloc(*in_buffer, (*buffer_size + d_http_payload_buffer_size)))) {
          residual_space += d_http_payload_buffer_size;
          memset((new_in_buffer + *payload_size), 0, residual_space);
          *in_buffer = new_in_buffer;
          *buffer_size += d_http_payload_buffer_size;
        } else
          result = SHIT_NO_MEMORY;
      } else
        residual_space = (*buffer_size - *payload_size);
      /* we have enough space now if we're still "NOICE" */
      if (result == NOICE) {
        size_t read_size = 0;
        if ((result = f_socket_read(descriptor, (*in_buffer + *payload_size), (*buffer_size - *payload_size), &read_size, timeout_milliseconds)) == NOICE) {
          coremio_result http_payload_unserialization_result = f_http_payload_unserialize(http_payload, (char *) *in_buffer, (*payload_size += read_size),
              shift_unserialized_payload_size);
          if (*shift_unserialized_payload_size > 0) {
            /* we have processed already some of the data in the buffer, let's get rid of it right now */
            memmove(*in_buffer, (*in_buffer + *shift_unserialized_payload_size), (*buffer_size - *shift_unserialized_payload_size));
            memset((*in_buffer + (*buffer_size - *shift_unserialized_payload_size)), 0, *shift_unserialized_payload_size);
            *payload_size -= *shift_unserialized_payload_size;
            *shift_unserialized_payload_size = 0;
          }
        }
      }
    } else
      result = SHIT_NOT_INITIALIZED;
  } while ((result == NOICE) && (http_payload->current_sequence_step != e_http_sequence_step_completed));
  return result;
}
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
static char *p_http_payload_unserialize_block(char *raw_payload, const char *needle, const size_t buffer_size, size_t *shift_unserialized_size,
    ssize_t *current_session_shift_size) {
  char *result = NULL;
  if (current_session_shift_size)
    *current_session_shift_size = -1;
  if (buffer_size > *shift_unserialized_size) {
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
coremio_result f_http_payload_unserialize(s_http_payload *http_payload, char *raw_payload, const size_t buffer_size, size_t *shift_unserialized_size) {
  coremio_result result = NOICE;
  if ((http_payload->current_sequence_step < e_http_sequence_step_completed) && (buffer_size > *shift_unserialized_size)) {
    char *raw_payload_active, *raw_payload_key_value, *raw_header_component;
    ssize_t current_session_shift_size;
    e_http_sequence_steps previous_sequence_step;
    do {
      previous_sequence_step = http_payload->current_sequence_step;
      switch (http_payload->current_sequence_step) {
        case e_http_sequence_step_A_header_block: {
          if ((raw_header_component = p_http_payload_unserialize_block(raw_payload, " ", buffer_size, shift_unserialized_size, NULL))) {
            if ((http_payload->enumerated_method = p_http_method_from_keyword(raw_header_component)) != e_http_method_undefined)
              http_payload->method = raw_header_component; /* it is a request */
            else
              http_payload->version = raw_header_component; /* it is a response */
            http_payload->current_sequence_step = e_http_sequence_step_B_header_block;
          }
          break;
        }
        case e_http_sequence_step_B_header_block: {
          if ((raw_header_component = p_http_payload_unserialize_block(raw_payload, " ", buffer_size, shift_unserialized_size, NULL))) {
            unsigned int status_code;
            if (http_payload->enumerated_method == e_http_method_undefined) {
              if (((status_code = atoi(raw_header_component)) >= 100) && (status_code < 600))
                http_payload->status_code = status_code;
              d_free(raw_header_component);
            } else
              http_payload->path = raw_header_component;
            http_payload->current_sequence_step = e_http_sequence_step_C_header_block;
          }
          break;
        }
        case e_http_sequence_step_C_header_block: {
          if (((raw_header_component = p_http_payload_unserialize_block(raw_payload, d_http_sequence_new_line_characters, buffer_size, shift_unserialized_size,
                    &current_session_shift_size)) == NULL) &&
              (current_session_shift_size >= 0)) {
            /* apparently, in the http request, the status message might be empty */
            http_payload->current_sequence_step = e_http_sequence_step_key_value;
          } else if (raw_header_component) {
            if (http_payload->enumerated_method != e_http_method_undefined)
              http_payload->version = raw_header_component;
            else
              http_payload->status_message = raw_header_component;
            http_payload->current_sequence_step = e_http_sequence_step_key_value;
          }
          break;
        }
        case e_http_sequence_step_key_value: {
          if (((raw_payload_key_value = p_http_payload_unserialize_block(raw_payload, d_http_sequence_new_line_characters, buffer_size, shift_unserialized_size,
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
            if ((buffer_size > *shift_unserialized_size) && ((buffer_size - *shift_unserialized_size) >= payload_length) &&
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
struct s_http_serialize_payload_container {
  unsigned char *buffer;
  size_t buffer_size, content_size;
};
static void p_http_payload_serialize_headers(s_http_payload_value_node *value_node, struct s_http_serialize_payload_container *serialized_payload_container) {
  size_t new_content_size = strlen(value_node->head.key) + 2 + strlen(value_node->value) + strlen(d_http_sequence_new_line_characters), additional_space = 0;
  coremio_result result = NOICE;
  while ((serialized_payload_container->content_size + new_content_size + 1) > (serialized_payload_container->buffer_size + additional_space))
    additional_space += d_http_payload_buffer_size;
  if (additional_space > 0) {
    unsigned char *new_buffer = (unsigned char *) d_realloc(serialized_payload_container->buffer,
        (serialized_payload_container->buffer_size + additional_space));
    if (new_buffer) {
      memset((new_buffer + serialized_payload_container->buffer_size), 0, additional_space);
      serialized_payload_container->buffer = new_buffer;
      serialized_payload_container->buffer_size += additional_space;
    } else
      result = SHIT_NO_MEMORY;
  }
  if ((result == NOICE) && (serialized_payload_container->buffer)) {
    snprintf((char *) (serialized_payload_container->buffer + serialized_payload_container->content_size), (new_content_size + 1), "%s: %s%s",
        value_node->head.key, value_node->value, d_http_sequence_new_line_characters);
    serialized_payload_container->content_size += new_content_size;
  }
}
coremio_result f_http_payload_serialize(s_http_payload *http_payload, unsigned char **raw_payload, size_t *buffer_size) {
  coremio_result result = NOICE;
  struct s_http_serialize_payload_container serialized_payload_container = {NULL, 0, 0};
  if (http_payload->current_sequence_step == e_http_sequence_step_completed) {
    const size_t new_line_size = strlen(d_http_sequence_new_line_characters);
    size_t version_size = ((http_payload->version) ? strlen(http_payload->version) : 0);
    if (http_payload->enumerated_method != e_http_method_undefined) {
      size_t method_size = ((http_payload->method) ? strlen(http_payload->method) : 0), path_size = ((http_payload->path) ? strlen(http_payload->path) : 0),
             additional_space = method_size + 1 + path_size + 1 + version_size + new_line_size;
      if ((serialized_payload_container.buffer = (unsigned char *) d_malloc(additional_space + 1))) {
        serialized_payload_container.buffer_size = (additional_space + 1);
        serialized_payload_container.content_size = additional_space;
        snprintf((char *) serialized_payload_container.buffer, (additional_space + 1), "%s %s %s%s", ((http_payload->method) ? http_payload->method : ""),
            ((http_payload->path) ? http_payload->path : ""), ((http_payload->version) ? http_payload->version : ""), d_http_sequence_new_line_characters);
        serialized_payload_container.buffer[additional_space] = 0;
      } else
        result = SHIT_NO_MEMORY;
    } else {
      size_t status_message_size = ((http_payload->status_message) ? strlen(http_payload->status_message) : 0),
             additional_space = version_size + 1 + 3 + 1 + status_message_size + new_line_size;
      if ((serialized_payload_container.buffer = (unsigned char *) d_malloc(additional_space + 1))) {
        serialized_payload_container.buffer_size = (additional_space + 1);
        serialized_payload_container.content_size = additional_space;
        snprintf((char *) serialized_payload_container.buffer, (additional_space + 1), "%s %u %s%s", ((http_payload->version) ? http_payload->version : ""),
            http_payload->status_code, ((http_payload->status_message) ? http_payload->status_message : ""), d_http_sequence_new_line_characters);
        serialized_payload_container.buffer[additional_space] = 0;
      } else
        result = SHIT_NO_MEMORY;
    }
    if (result == NOICE) {
      s_http_payload_value_node *content_length_value_node = NULL;
      ssize_t payload_size = 0;
      f_dictionary_foreach(&(http_payload->configuration), (l_dictionary_node_visit) p_http_payload_serialize_headers, &(serialized_payload_container));
      if ((content_length_value_node = (s_http_payload_value_node *) f_dictionary_get_if_exists(&(http_payload->configuration), "content-length")))
        if ((payload_size = ((content_length_value_node->value) ? atoi(content_length_value_node->value) : 0)) < 0)
          payload_size = 0;
      if (payload_size > 0) {
        size_t additional_space = 0;
        if ((serialized_payload_container.content_size + payload_size + new_line_size) > (serialized_payload_container.buffer_size))
          additional_space = ((serialized_payload_container.content_size + payload_size + new_line_size) - serialized_payload_container.buffer_size);
        if (additional_space > 0) {
          unsigned char *new_buffer = (unsigned char *) d_realloc(serialized_payload_container.buffer,
              (serialized_payload_container.buffer_size + additional_space));
          if (new_buffer) {
            serialized_payload_container.buffer = new_buffer;
            serialized_payload_container.buffer_size += additional_space;
          } else
            result = SHIT_NO_MEMORY;
        }
        if (result == NOICE) {
          strncpy((char *) (serialized_payload_container.buffer + serialized_payload_container.content_size), d_http_sequence_new_line_characters,
              new_line_size);
          memcpy((serialized_payload_container.buffer + serialized_payload_container.content_size + new_line_size), http_payload->body, payload_size);
          serialized_payload_container.content_size += (new_line_size + payload_size);
          serialized_payload_container.buffer[serialized_payload_container.content_size] = 0;
        }
      }
      if (result == NOICE) {
        *raw_payload = serialized_payload_container.buffer;
        *buffer_size = serialized_payload_container.content_size;
      }
    }
  } else
    result = SHIT_NOT_INITIALIZED;
  if ((result != NOICE) && (serialized_payload_container.buffer))
    d_free(serialized_payload_container.buffer);
  return result;
}
void f_http_payload_free(s_http_payload *http_payload) {
  http_payload->current_sequence_step = e_http_sequence_step_A_header_block;
  if (http_payload->method)
    d_free(http_payload->method);
  if (http_payload->version)
    d_free(http_payload->version);
  if (http_payload->path)
    d_free(http_payload->path);
  if (http_payload->status_message)
    d_free(http_payload->status_message);
  if (http_payload->body)
    d_free(http_payload->body);
  f_dictionary_free(&(http_payload->configuration));
}
