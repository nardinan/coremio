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
#ifndef COREMIO_LISP_H
#define COREMIO_LISP_H
#define d_lisp_array_bucket_size 2
#include "tokens.h"
#include "red_black_tree.h"
typedef enum e_lisp_node_type {
  e_lisp_node_atom_undefined = 0,
  e_lisp_node_atom_symbol, /* a token of type 'word' or 'symbol' */
  e_lisp_node_atom_token, /* every other kind of token */
  e_lisp_node_list,
  e_lisp_node_atom_function
} e_lisp_node_type;
typedef struct s_lisp_node {
  s_list_node head;
  e_lisp_node_type type;
  union {
    s_token *token;
    s_list list;
  } value;
} s_lisp_node;
typedef struct s_lisp_environment_node {
  s_red_black_tree_node head;
  s_token *symbol;
  s_lisp_node *value;
} s_lisp_environment_node;
typedef struct s_lisp_environment {
  s_red_black_tree symbols;
  struct s_lisp_environment *parent;
} s_lisp_environment;
typedef struct s_lisp {
  s_lisp_node *root_code;
  s_lisp_environment *root_environment;
  s_list tokens;
} s_lisp;
extern coremio_result f_lisp_environment_explode_buffer(const char *buffer, s_lisp *lisp);
extern coremio_result f_lisp_environment_explode_stream(int stream, s_lisp *lisp);
extern void f_lisp_free(s_lisp *lisp);
void f_lisp_print_plain(int stream, s_lisp_node *root);
#endif //COREMIO_LISP_H
