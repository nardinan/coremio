/**
 * MIT License
 * Copyright (c) [2022] The Barfing Fox [Andrea Nardinocchi (andrea@nardinan.it)]
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
#include "../include/coremio/json.h"
const char *m_json_types[] = {
  "undefined",
  "value",
  "array",
  "object",
  "", // __STOOPIDITY RESERVED__
  "", // __STOOPIDITY RESERVED__
  ""  // __STOOPIDITY RESERVED__
};
static s_json_node *p_json_get_node_args(s_json *json, s_json_node *starting_node, const char *format, va_list parameters, const bool create) {
  s_json_node *result = NULL;
  if ((starting_node) || ((starting_node = json->root))) {
    if (*format) {
      s_json_node *next_starting_node = NULL;
      size_t index_array = 0;
      char *current_label = NULL;
      bool wrong_type = false;
      switch (*format) {
        case 's':
          if ((current_label = va_arg(parameters, char *))) {
            if (starting_node->type == e_json_type_object) {
              s_json_node *next_node = (s_json_node *)(starting_node->content.children.head);
              while ((next_node) && (next_node->key) && (!d_token_string_compare(next_node->key, current_label)))
                next_node = (s_json_node *)next_node->head.next;
              next_starting_node = next_node;
            } else {
              fprintf(stderr, "[JSON] Object expected (looking for entry '%s'), but it is a %s\n", current_label, m_tokens_types[starting_node->type]);
              wrong_type = true;
            }
          }
          break;
        case 'd':
          index_array = va_arg(parameters, long);
          /* we've been asked to check an entry of the starting node so, we are expecting an array. If the starting node is an object, we can promote it to
           * an array if the following conditions are satisfied:
           * - no value associated
           * - no children
           */
          if (starting_node->type == e_json_type_object)
            if ((!starting_node->content.value) && (!starting_node->content.children.entries))
              starting_node->type = e_json_type_array;
          if (starting_node->type == e_json_type_array) {
            s_json_node *next_node = (s_json_node *)(starting_node->content.children.head);
            for (size_t index = 0; ((next_node) && (index < index_array)); ++index)
              next_node = (s_json_node *)next_node->head.next;
            next_starting_node = next_node;
          } else {
            fprintf(stderr, "[JSON] Array expected (looking for entry with index %zu), but it is a %s\n", index_array, m_tokens_types[starting_node->type]);
            wrong_type = true;
          }
          break;
        default:
          break;
      }
      if (!wrong_type)
        if ((next_starting_node) || ((create) && ((next_starting_node = f_json_new_node(current_label, starting_node, e_json_type_object)))))
          result = p_json_get_node_args(json, next_starting_node, (format + 1), parameters, create);
    } else
      result = starting_node;
  }
  return result;
}
s_json_node *f_json_get_node(s_json *json, s_json_node *starting_node, const char *format, ...) {
  va_list parameters;
  s_json_node *result;
  va_start(parameters, format);
  {
    result = p_json_get_node_args(json, starting_node, format, parameters, false);
  }
  va_end(parameters);
  return result;
}
s_json_node *f_json_get_node_or_create(s_json *json, s_json_node *starting_node, const char *format, ...) {
  va_list parameters;
  s_json_node *result;
  va_start(parameters, format);
  {
    result = p_json_get_node_args(json, starting_node, format, parameters, true);
  }
  va_end(parameters);
  return result;
}
static void p_json_delete_node_args(s_json *json, s_json_node *starting_node, const char *format, const va_list parameters) {
  s_json_node *selected_node = p_json_get_node_args(json, starting_node, format, parameters, false);
  if (selected_node) {
    f_list_remove_from_owner((s_list_node *)selected_node);
    f_json_free_node(selected_node);
  }
}
void f_json_delete_node(s_json *json, s_json_node *starting_node, const char *format, ...) {
  va_list parameters;
  va_start(parameters, format);
  {
    p_json_delete_node_args(json, starting_node, format, parameters);
  }
  va_end(parameters);
}
double f_json_get_value(s_json *json, s_json_node *starting_node, const char *format, ...) {
  va_list parameters;
  double result = 0;
  va_start(parameters, format);
  {
    s_json_node *current_node;
    if (((current_node = p_json_get_node_args(json, starting_node, format, parameters, false))) && (current_node->type == e_json_type_value) &&
        (current_node->content.value->type == e_token_type_value))
      result = current_node->content.value->token.token_value;
  }
  va_end(parameters);
  return result;
}
char *f_json_get_char(s_json *json, s_json_node *starting_node, const char *format, ...) {
  va_list parameters;
  char *result = NULL;
  va_start(parameters, format);
  {
    s_json_node *current_node;
    if (((current_node = p_json_get_node_args(json, starting_node, format, parameters, false))) && (current_node->type == e_json_type_value) &&
        ((current_node->content.value->type == e_token_type_string) || (current_node->content.value->type == e_token_type_word)))
      result = current_node->content.value->token.token_char;
  }
  va_end(parameters);
  return result;
}
bool f_json_get_bool(s_json *json, s_json_node *starting_node, const char *format, ...) {
  va_list parameters;
  bool result = false;
  va_start(parameters, format);
  {
    s_json_node *current_node;
    if (((current_node = p_json_get_node_args(json, starting_node, format, parameters, false))) && (current_node->type == e_json_type_value) &&
        ((current_node->content.value->type == e_token_type_string) || (current_node->content.value->type == e_token_type_word)))
      if (strcasecmp(current_node->content.value->token.token_char, "true") == 0)
        result = true;
  }
  va_end(parameters);
  return result;
}
static coremio_result p_json_set_token(s_json *json, s_token *token, s_json_node *starting_node, const char *format, const va_list parameters) {
  coremio_result result = NOICE;
  s_json_node *holder_node;
  if ((holder_node = p_json_get_node_args(json, starting_node, format, parameters, true))) {
    /* we've been asked to modify the content of a node so, we are expecting a value. If the holder node is an object or an array, we can
     * promote it to a value if the following conditions are satisfied:
     * - no value associated
     * - no children
     */
    if ((holder_node->type == e_json_type_object) || (holder_node->type == e_json_type_array))
      if ((!holder_node->content.value) && (!holder_node->content.children.entries))
        holder_node->type = e_json_type_value;
    if (holder_node->type == e_json_type_value) {
      if ((holder_node->content.value) && (holder_node->value_allocated))
        f_tokens_free_token(holder_node->content.value);
      holder_node->content.value = token;
      holder_node->value_allocated = 1;
    } else
      result = SHIT_INVALID_PARAMETERS;
  } else
    result = SHIT_NOT_FOUND;
  return result;
}
coremio_result f_json_set_value(s_json *json, const double value, s_json_node *starting_node, const char *format, ...) {
  coremio_result result;
  va_list parameters;
  va_start(parameters, format);
  {
    s_token *token = f_tokens_new_token_value(value);
    if ((result = p_json_set_token(json, token, starting_node, format, parameters)) != NOICE)
      f_tokens_free_token(token);
  }
  va_end(parameters);
  return result;
}
coremio_result f_json_set_char(s_json *json, const char *value, s_json_node *starting_node, const char *format, ...) {
  coremio_result result;
  va_list parameters;
  va_start(parameters, format);
  {
    s_token *token = f_tokens_new_token_char(value, e_token_type_string);
    if ((result = p_json_set_token(json, token, starting_node, format, parameters)) != NOICE)
      f_tokens_free_token(token);
  }
  va_end(parameters);
  return result;
}
coremio_result f_json_set_bool(s_json *json, const bool value, s_json_node *starting_node, const char *format, ...) {
  coremio_result result;
  va_list parameters;
  va_start(parameters, format);
  {
    s_token *token = f_tokens_new_token_char(((value)?"true":"false"), e_token_type_string);
    if ((result = p_json_set_token(json, token, starting_node, format, parameters)) != NOICE)
      f_tokens_free_token(token);
  }
  va_end(parameters);
  return result;
}
static s_token *p_json_explode_add_value(s_token *current_token, s_json_node **json_node, const bool ignore_key) {
  enum e_scoped_json_actions {
    e_scoped_json_action_key,
    e_scoped_json_action_separator,
    e_scoped_json_action_value,
    e_scoped_json_action_terminated
  } current_action = e_scoped_json_action_key;
  s_token *key_token = NULL;
  if (ignore_key)
    current_action = e_scoped_json_action_value;
  while ((current_action != e_scoped_json_action_terminated) && (current_token)) {
    switch (current_action) {
      case e_scoped_json_action_key:
        if ((current_token->type == e_token_type_string) ||
            (current_token->type == e_token_type_word)) {
          key_token = current_token;
          current_action = e_scoped_json_action_separator;
        }
        break;
      case e_scoped_json_action_separator:
        if ((current_token->type == e_token_type_symbol) &&
            (current_token->token.token_symbol == ':'))
          current_action = e_scoped_json_action_value;
        break;
      case e_scoped_json_action_value:
        if ((current_token->type == e_token_type_word) ||
            (current_token->type == e_token_type_string) ||
            (current_token->type == e_token_type_value)) {
          if ((*json_node = (s_json_node *)d_malloc(sizeof(s_json_node)))) {
            memset(*json_node, 0, sizeof(s_json_node));
            (*json_node)->key = key_token;
            (*json_node)->type = e_json_type_value;
            (*json_node)->content.value = current_token;
          }
          current_action = e_scoped_json_action_terminated;
        } else if ((current_token->type == e_token_type_symbol) &&
                   ((current_token->token.token_symbol == '{') || (current_token->token.token_symbol == '['))) {
          const char bootstrap_symbol = current_token->token.token_symbol;
          if ((*json_node = (s_json_node *)d_malloc(sizeof(s_json_node)))) {
            char termination_symbol = 0;
            memset(*json_node, 0, sizeof(s_json_node));
            (*json_node)->key = key_token;
            if (bootstrap_symbol == '{') {
              (*json_node)->type = e_json_type_object;
              termination_symbol = '}';
            } else if (bootstrap_symbol == '[') {
              (*json_node)->type = e_json_type_array;
              termination_symbol = ']';
            }
            current_token = (s_token *)current_token->head.next;
            while ((current_token) && ((current_token->type != e_token_type_symbol) || (current_token->token.token_symbol != termination_symbol))) {
              s_json_node *child = NULL;
              current_token = p_json_explode_add_value(current_token, &child, ((*json_node)->type == e_json_type_array));
              if (child) {
                child->owner = (*json_node);
                f_list_append(&((*json_node)->content.children), (s_list_node *)child, e_list_insert_tail);
              }
            }
          }
          current_action = e_scoped_json_action_terminated;
        }
      default:
        break;
    }
    if (current_token)
      current_token = (s_token *)current_token->head.next;
  }
  return current_token;
}
coremio_result f_json_explode_buffer(const char *buffer, s_json *json) {
  coremio_result result;
  size_t line_accumulator = 0, character_accumulator = 0;
  memset(json, 0, sizeof(s_json));
  if ((result = f_tokens_explode_buffer(buffer, "{}[]:,", NULL, " \n\r\t", &line_accumulator, &character_accumulator, &(json->tokens))) == NOICE)
    p_json_explode_add_value((s_token *)json->tokens.head, &(json->root), true);
  return result;
}
coremio_result f_json_explode_stream(const int stream, s_json *json) {
  coremio_result result;
  memset(json, 0, sizeof(s_json));
  if ((result = f_tokens_explode_stream(stream, "{}[]:,", NULL, " \n\r\t", &(json->tokens))) == NOICE)
    p_json_explode_add_value((s_token *)json->tokens.head, &(json->root), true);
  return result;
}
void f_json_initialize_empty(s_json *json) {
  memset(json, 0, sizeof(s_json));
  json->root = f_json_new_node(NULL, NULL, e_json_type_object);
}
static void p_json_dump_token(const int stream, const s_token *token, bool as_string) {
  if (token) {
    if ((as_string) || ((as_string = (token->type == e_token_type_string))))
      write(stream, "\"", 1);
    switch (token->type) {
      case e_token_type_word:
      case e_token_type_string:
        dprintf(stream, "%s", token->token.token_char);
        break;
      case e_token_type_value:
        dprintf(stream, "%f", token->token.token_value);
        break;
      case e_token_type_symbol:
        dprintf(stream, "%c", token->token.token_symbol);
        break;
      default:
        break;
    }
    if (as_string)
      write(stream, "\"", 1);
  }
}
void f_json_free_node(s_json_node *node) {
  if ((node->type == e_json_type_object) || (node->type == e_json_type_array)) {
    s_json_node *current_node;
    while ((current_node = (s_json_node *)node->content.children.head)) {
      f_list_remove(&(node->content.children), node->content.children.head);
      f_json_free_node(current_node);
    }
  } else if (node->type == e_json_type_value) {
    if ((node->content.value) && (node->value_allocated)) {
      if ((node->content.value->allocated) && (node->content.value->token.token_char))
        d_free(node->content.value->token.token_char);
      d_free(node->content.value);
    }
  }
  if ((node->key) && (node->key_allocated)) {
    if ((node->key->allocated) && (node->key->token.token_char))
      d_free(node->key->token.token_char);
    d_free(node->key);
  }
  d_free(node);
}
void f_json_free(s_json *json) {
  f_json_free_node(json->root);
  f_tokens_free(&(json->tokens));
}
void f_json_print_plain(const int stream, const s_json_node *starting_node, s_json *json) {
  if ((starting_node) || ((starting_node = json->root))) {
    s_json_node *next_starting_node;
    if (starting_node->key) {
      p_json_dump_token(stream, starting_node->key, true);
      write(stream, ":", 1);
    }
    switch (starting_node->type) {
      case e_json_type_value:
        p_json_dump_token(stream, starting_node->content.value, false);
        break;
      case e_json_type_array:
        write(stream, "[", 1);
        d_list_foreach(&(starting_node->content.children), next_starting_node, s_json_node) {
          f_json_print_plain(stream, next_starting_node, json);
          if (next_starting_node->head.next)
            write(stream, ",", 1);
        }
        write(stream, "]", 1);
        break;
      case e_json_type_object:
        write(stream, "{", 1);
        d_list_foreach(&(starting_node->content.children), next_starting_node, s_json_node) {
          f_json_print_plain(stream, next_starting_node, json);
          if (next_starting_node->head.next)
            write(stream, ",", 1);
        }
        write(stream, "}", 1);
        break;
      default:
        break;
    }
  }
}
s_json_node *f_json_new_node(const char *label, s_json_node *container, const e_json_types type) {
  s_json_node *result = NULL;
  if ((result = (s_json_node *)d_malloc(sizeof(s_json_node)))) {
    memset(result, 0, sizeof(s_json_node));
    if ((!label) || ((result->key = f_tokens_new_token_char((char *)label, e_token_type_string)))) {
      result->key_allocated = 1;
      result->type = type;
      if ((container) && ((container->type == e_json_type_object) || (container->type == e_json_type_array)))
        f_list_append(&(container->content.children), (s_list_node *)result, e_list_insert_tail);
    } else {
      d_free(result);
      result = NULL;
    }
  }
  return result;
}