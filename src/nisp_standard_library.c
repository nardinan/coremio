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
#include "../include/coremio/nisp_standard_library.h"
#include "../include/coremio/nisp.h"
s_nisp_node *f_nisp_standard_library_divide(s_nisp *nisp, s_nisp_environment *environment) {
  const s_nisp_node_environment *value_a = f_nisp_lookup_environment_label("a", environment),
  *value_b = f_nisp_lookup_environment_label("b", environment);
  s_nisp_node *result = NULL;
  if ((d_nisp_is_token_numeric(value_a->value)) && (d_nisp_is_token_numeric(value_b->value)))
    result = f_nisp_generate_node_from_token(nisp,
      f_tokens_new_token_value(value_a->value->value.token->token.token_value / value_b->value->value.token->token.token_value));
  else
    fprintf(stderr, "error <%s> as we got a division of non-numeric values\n", __FUNCTION__);
  return result;
}
s_nisp_node *f_nisp_standard_library_multiply(s_nisp *nisp, s_nisp_environment *environment) {
  const s_nisp_node_environment *value_a = f_nisp_lookup_environment_label("a", environment),
  *value_b = f_nisp_lookup_environment_label("b", environment);
  s_nisp_node *result = NULL;
  if ((d_nisp_is_token_numeric(value_a->value)) && (d_nisp_is_token_numeric(value_b->value)))
    result = f_nisp_generate_node_from_token(nisp,
      f_tokens_new_token_value(value_a->value->value.token->token.token_value * value_b->value->value.token->token.token_value));
  else
    fprintf(stderr, "error <%s> as we got a multiplication of non-numeric values\n", __FUNCTION__);
  return result;
}
s_nisp_node *f_nisp_standard_library_sum(s_nisp *nisp, s_nisp_environment *environment) {
  const s_nisp_node_environment *value_a = f_nisp_lookup_environment_label("a", environment),
  *value_b = f_nisp_lookup_environment_label("b", environment);
  s_nisp_node *result = NULL;
  if ((d_nisp_is_token_numeric(value_a->value)) && (d_nisp_is_token_numeric(value_b->value)))
    result = f_nisp_generate_node_from_token(nisp,
      f_tokens_new_token_value(value_a->value->value.token->token.token_value + value_b->value->value.token->token.token_value));
  else
    fprintf(stderr, "error <%s> as we got a sum of non-numeric values\n", __FUNCTION__);
  return result;
}
s_nisp_node *f_nisp_standard_library_subtract(s_nisp *nisp, s_nisp_environment *environment) {
  const s_nisp_node_environment *value_a = f_nisp_lookup_environment_label("a", environment),
  *value_b = f_nisp_lookup_environment_label("b", environment);
  s_nisp_node *result = NULL;
  if ((d_nisp_is_token_numeric(value_a->value)) && (d_nisp_is_token_numeric(value_b->value)))
    result = f_nisp_generate_node_from_token(nisp,
      f_tokens_new_token_value(value_a->value->value.token->token.token_value - value_b->value->value.token->token.token_value));
  else
    fprintf(stderr, "error <%s> as we got a subtraction of non-numeric values\n", __FUNCTION__);
  return result;
}
struct s_nisp_node *f_nisp_standard_library_equal(s_nisp *nisp, s_nisp_environment *environment) {
  const s_nisp_node_environment *value_a = f_nisp_lookup_environment_label("a", environment),
  *value_b = f_nisp_lookup_environment_label("b", environment);
  s_nisp_node *result = NULL;
  if ((((d_nisp_is_symbol(value_a->value)) && (d_nisp_is_symbol(value_b->value)))) ||
      ((d_nisp_is_token(value_a->value)) && (d_nisp_is_token(value_b->value)))) {
    if (f_tokens_compare(value_a->value->value.token, value_b->value->value.token))
      result = nisp->true_symbol;
    else
      result = nisp->false_symbol;
  } else
    fprintf(stderr, "error <%s> as we got to compare two values of different type or non-basic types\n", __FUNCTION__);
  return result;
}
static void p_nisp_standard_library_print_raw(s_nisp_node *node) {
  switch (node->type) {
    case e_nisp_node_atom_token: {
      f_tokens_print_plain(STDOUT_FILENO, node->value.token);
      break;
    }
    case e_nisp_node_list: {
      s_nisp_node *current_node;
      d_list_foreach(&(node->value.list), current_node, s_nisp_node) {
        p_nisp_standard_library_print_raw(current_node);
      }
      break;
    }
    default: {}
  }
}
s_nisp_node *f_nisp_standard_library_println(s_nisp *nisp, s_nisp_environment *environment) {
  const s_nisp_node_environment *value_list = f_nisp_lookup_environment_label("quoted_list", environment);
  s_nisp_node *current = value_list->value;
  p_nisp_standard_library_print_raw(current);
  write(STDOUT_FILENO, "\n", 1);
  return NULL;
}
s_nisp_node *f_nisp_standard_library_print(s_nisp *nisp, s_nisp_environment *environment) {
  const s_nisp_node_environment *value_list = f_nisp_lookup_environment_label("quoted_list", environment);
  s_nisp_node *current = value_list->value;
  p_nisp_standard_library_print_raw(current);
  return NULL;
}