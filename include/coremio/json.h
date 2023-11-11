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
#ifndef COREMIO_JSON_H
#define COREMIO_JSON_H
#include "tokens.h"
#define d_json_node_is_string(n) ((n)&&((n)->type == e_json_type_value)&&((n)->content.value)&&\
  (((n)->content.value->type==e_token_type_string)||((n)->content.value->type==e_token_type_word)))
#define d_json_node_is_value(n) ((n)&&((n)->type == e_json_type_value)&&((n)->content.value)&&\
  (((n)->content.value->type==e_token_type_value)))
typedef enum e_json_types {
  e_json_type_undefined = 0,
  e_json_type_value,
  e_json_type_array,
  e_json_type_object
} e_json_types;
extern const char *m_json_types[];
typedef struct s_json_node {
  s_list_node head;
  unsigned char key_allocated: 1, value_allocated: 1;
  struct s_json_node *owner;
  s_token *key;
  e_json_types type;
  union {
    s_token *value;
    s_list children;
  } content;
} s_json_node;
typedef struct s_json {
  s_json_node *root;
  s_list tokens;
} s_json;
extern s_json_node *f_json_get_node(s_json *json, s_json_node *starting_node, const char *format, ...);
extern s_json_node *f_json_get_node_or_create(s_json *json, s_json_node *starting_node, const char *format, ...);
extern void f_json_delete_node(s_json *json, s_json_node *starting_node, const char *format, ...);
extern double f_json_get_value(s_json *json, s_json_node *starting_node, const char *format, ...);
extern char *f_json_get_char(s_json *json, s_json_node *starting_node, const char *format, ...);
extern bool f_json_get_bool(s_json *json, s_json_node *starting_node, const char *format, ...);
extern coremio_result f_json_set_value(s_json *json, double value, s_json_node *starting_node, const char *format, ...);
extern coremio_result f_json_set_char(s_json *json, const char *value, s_json_node *starting_node, const char *format, ...);
extern coremio_result f_json_set_bool(s_json *json, bool value, s_json_node *starting_node, const char *format, ...);
extern coremio_result f_json_explode_buffer(const char *buffer, s_json *json);
extern coremio_result f_json_explode_stream(int stream, s_json *json);
extern void f_json_initialize_empty(s_json *json);
extern void f_json_free_node(s_json_node *node);
extern void f_json_free(s_json *json);
extern void f_json_print_plain(int stream, const s_json_node *starting_node, s_json *json);
extern s_json_node *f_json_new_node(const char *label, s_json_node *container, e_json_types type);
#endif //COREMIO_JSON_H
