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
#ifndef COREMIO_NISP_H
#define COREMIO_NISP_H
#include "tokens.h"
#include "dictionary.h"
#define d_nisp_next(n) ((s_nisp_node *)((s_list_node *)(n))->next)
#define d_nisp_is_numeric(n) ((n)&&((n)->type==e_nisp_node_atom_token)&&((n)->value.token->type==e_token_type_value))
#define d_nisp_is_string(n) ((n)&&((n)->type==e_nisp_node_atom_token)&&((n)->value.token->type==e_token_type_string))
typedef enum e_nisp_node_type {
  e_nisp_node_atom_undefined = 0,
  e_nisp_node_atom_symbol, /* a token of type 'word' or 'symbol' */
  e_nisp_node_atom_token,  /* every other kind of token */
  e_nisp_node_list,
  e_nisp_node_lambda,
  e_nisp_node_native_symbol,
  e_nisp_node_native_lambda
} e_nisp_node_type;
struct s_nisp_environment;
struct s_nisp_node;
struct s_nisp;
typedef struct s_nisp_node *(l_nisp_native_lambda)(struct s_nisp *, struct s_nisp_environment *);
typedef struct s_nisp_node {
  s_list_node head;
  e_nisp_node_type type;
  unsigned char mark: 1, token_from_lexer: 1; /* if the token associated comes from the lexer, shouldn't be deleted */
  union {
    s_token *token;
    s_list list;
    struct {
      s_list list;
      l_nisp_native_lambda *routine;
    } native_lambda;
  } value;
  size_t line_definition;
} s_nisp_node;
typedef struct s_nisp_node_garbage_collector {
  s_list_node head;
  s_nisp_node node;
} s_nisp_node_garbage_collector;
typedef struct s_nisp_node_environment {
  s_dictionary_node head;
  s_nisp_node *value;
} s_nisp_node_environment;
typedef struct s_nisp_environment {
  s_dictionary symbols;
  struct s_nisp_environment *parent;
} s_nisp_environment;
typedef struct s_nisp {
  s_nisp_node *root_code;
  s_nisp_environment root_environment;
  s_list tokens, garbage_collector;
} s_nisp;
extern s_nisp_node *f_nisp_generate_node(s_nisp *nisp, e_nisp_node_type type);
extern s_nisp_node *f_nisp_generate_node_from_token(s_nisp *nisp, s_token *token);
extern void f_nisp_append_native_lambda(s_nisp *nisp, const char *symbol, char *parameters[], l_nisp_native_lambda *routine);
extern s_nisp_node_environment *f_nisp_lookup_environment_label(const char *symbol, s_nisp_environment *environment);
extern s_nisp_node_environment *f_nisp_lookup_environment_node(const s_nisp_node *symbol, s_nisp_environment *environment);
extern coremio_result f_nisp_environment_explode_buffer(const char *buffer, s_nisp *nisp);
extern coremio_result f_nisp_environment_explode_stream(int stream, s_nisp *nisp);
extern coremio_result f_nisp_execute(s_nisp *nisp);
extern void f_nisp_mark(s_nisp *nisp);
extern void f_nisp_sweep(s_nisp *nisp);
extern void f_nisp_free(s_nisp *nisp);
extern void f_nisp_print_nodes_plain(int stream, const s_nisp_node *root);
extern void f_nisp_print_environment_plain(int stream, const s_nisp_environment *environment);
#endif //COREMIO_NISP_H
