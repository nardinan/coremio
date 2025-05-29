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
#include "dictionary.h"
#define d_lisp_next(n) ((s_lisp_node *)((s_list_node *)(n))->next)
#define d_lisp_is_numeric(n) ((n)&&((n)->type==e_lisp_node_atom_token)&&((n)->value.token->type==e_token_type_value))
#define d_lisp_is_string(n) ((n)&&((n)->type==e_lisp_node_atom_token)&&((n)->value.token->type==e_token_type_string))
typedef enum e_lisp_node_type {
  e_lisp_node_atom_undefined = 0,
  e_lisp_node_atom_symbol, /* a token of type 'word' or 'symbol' */
  e_lisp_node_atom_token, /* every other kind of token */
  e_lisp_node_list,
  e_lisp_node_lambda,
  e_lisp_node_native_lambda
} e_lisp_node_type;
struct s_lisp_environment;
struct s_lisp_node;
struct s_lisp;
typedef struct s_lisp_node *(l_lisp_native_lambda)(struct s_lisp *, struct s_lisp_environment *);
typedef struct s_lisp_node {
  s_list_node head;
  e_lisp_node_type type;
  unsigned char mark: 1, token_from_lexer: 1; /* if the token associated comes from the lexer, shouldn't be deleted */
  union {
    s_token *token;
    s_list list;
    struct {
      s_list list;
      l_lisp_native_lambda *routine;
    } native_lambda;
  } value;
  size_t line_definition;
} s_lisp_node;
typedef struct s_lisp_node_garbage_collector {
  s_list_node head;
  s_lisp_node *node;
} s_lisp_node_garbage_collector;
typedef struct s_lisp_environment_node {
  s_dictionary_node head;
  s_lisp_node *value;
} s_lisp_environment_node;
typedef struct s_lisp_environment {
  s_dictionary symbols;
  struct s_lisp_environment *parent;
} s_lisp_environment;
typedef struct s_lisp {
  s_lisp_node *root_code;
  s_lisp_environment root_environment;
  s_list tokens, garbage_collector;
} s_lisp;
extern s_lisp_node *f_lisp_generate_node(s_lisp *lisp, e_lisp_node_type type);
extern s_lisp_node *f_lisp_generate_atom(s_lisp *lisp, s_token *token);
extern void f_lisp_append_native_lambda(s_lisp *lisp, const char *symbol, char *parameters[], l_lisp_native_lambda *routine);
extern s_lisp_environment_node *f_lisp_lookup_environment_label(const char *symbol, s_lisp_environment *environment);
extern s_lisp_environment_node *f_lisp_lookup_environment_node(const s_lisp_node *symbol, s_lisp_environment *environment);
extern coremio_result f_lisp_environment_explode_buffer(const char *buffer, s_lisp *lisp);
extern coremio_result f_lisp_environment_explode_stream(int stream, s_lisp *lisp);
extern coremio_result f_lisp_execute(s_lisp *lisp);
extern void f_lisp_mark(s_lisp *lisp);
extern void f_lisp_sweep(s_lisp *lisp);
extern void f_lisp_free(s_lisp *lisp);
extern void f_lisp_print_nodes_plain(int stream, const s_lisp_node *root);
extern void f_lisp_print_environment_plain(int stream, const s_lisp_environment *environment);
#endif //COREMIO_LISP_H
