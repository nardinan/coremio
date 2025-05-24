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
#include "../include/coremio/lisp.h"
const char *m_lisp_node_types[] = {
  "undefined",
  "symbol",
  "token",
  "list",
  "function",
  "", // __STOOPIDITY RESERVED__
  "", // __STOOPIDITY RESERVED__
  ""  // __STOOPIDITY RESERVED__
};
static s_lisp_node *p_lisp_generate_atom(s_token *token) {
  s_lisp_node *result = NULL;
  if ((result = (s_lisp_node *)d_malloc(sizeof(s_lisp_node)))) {
    memset(result, 0, sizeof(s_lisp_node));
    if ((token->type == e_token_type_word) ||
      (token->type == e_token_type_symbol)) {
      result->type = e_lisp_node_atom_symbol;
      } else {
        result->type = e_lisp_node_atom_token;
      }
    result->value.token = token;
  }
  return result;
}
static s_lisp_node *p_lisp_generate_abstract_syntax_tree(s_token **token) {
  s_lisp_node *result = NULL;
  if (*token) {
    if ((result = (s_lisp_node *)d_malloc(sizeof(s_lisp_node)))) {
      ssize_t ignore_line = -1;
      bool end_of_scope = false;
      memset(result, 0, sizeof(s_lisp_node));
      result->type = e_lisp_node_list;
      while ((!end_of_scope) && (*token)) {
        s_token *current_token = *token;
        *token = (s_token *)((s_list_node *)(*token))->next;
        if (ignore_line != current_token->line) {
          if ((current_token->type == e_token_type_symbol) &&  (current_token->token.token_symbol == ';')) {
            ignore_line = (ssize_t)current_token->line;
          } else if ((current_token->type == e_token_type_symbol) &&  (current_token->token.token_symbol == '(')) {
            s_lisp_node *lisp_node = p_lisp_generate_abstract_syntax_tree(token);
            if (lisp_node)
              f_list_append(&(result->value.list), (s_list_node *)lisp_node, e_list_insert_tail);
          } else if ((current_token->type == e_token_type_symbol) &&  (current_token->token.token_symbol == ')')) {
            end_of_scope = true;
          } else {
            s_lisp_node *lisp_node = p_lisp_generate_atom(current_token);
            if (lisp_node)
              f_list_append(&(result->value.list), (s_list_node *)lisp_node, e_list_insert_tail);
          }
        }
      }
    }
  }
  return result;
}
coremio_result f_lisp_environment_explode_buffer(const char *buffer, s_lisp *lisp) {
  coremio_result result;
  size_t line_accumulator = 0, character_accumulator = 0;
  memset(lisp, 0, sizeof(s_lisp));
  if ((result = f_tokens_explode_buffer(buffer, "();", NULL, " \n\r\t", &line_accumulator, &character_accumulator, &(lisp->tokens))) == NOICE) {
    s_token *current_token = (s_token *)lisp->tokens.head;
    lisp->root_code = p_lisp_generate_abstract_syntax_tree(&current_token);
  }
  return result;
}
coremio_result f_lisp_environment_explode_stream(int stream, s_lisp *lisp) {
  coremio_result result;
  memset(lisp, 0, sizeof(s_lisp));
  if ((result = f_tokens_explode_stream(stream, "();", NULL, " \n\r\t", &(lisp->tokens))) == NOICE) {
    s_token *current_token = (s_token *)lisp->tokens.head;
    lisp->root_code = p_lisp_generate_abstract_syntax_tree(&current_token);
  }
  return result;
}
void f_lisp_free(s_lisp *lisp) {
  f_tokens_free(&(lisp->tokens));
}
void p_lisp_print_plain(int stream, s_lisp_node *root, size_t level) {
  if (root) {
    for (size_t index = 0; index < level; ++index)
      write(stream, "| ", 2);
    dprintf(stream, "| - [type: %s]", m_lisp_node_types[root->type]);
    if (root->type != e_lisp_node_list)
      f_tokens_print_detailed(stream, root->value.token);
    write(stream, "\n", 1);
    if (root->type == e_lisp_node_list) {
      s_lisp_node *current_node;
      d_list_foreach(&(root->value.list), current_node, s_lisp_node)
        p_lisp_print_plain(stream, current_node, (level + 1));
    }
  }
}
void f_lisp_print_plain(int stream, s_lisp_node *root) {
  p_lisp_print_plain(stream, root, 0);
}