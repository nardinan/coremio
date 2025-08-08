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
#include "../include/coremio/nisp.h"
#include "../include/coremio/nisp_standard_library.h"
const char *m_nisp_node_types[] = {
  "undefined",
  "symbol",
  "token",
  "list",
  "lambda",
  "native_symbol",
  "native_lambda",
  "", // __STOOPIDITY RESERVED__
  "", // __STOOPIDITY RESERVED__
  ""  // __STOOPIDITY RESERVED__
};
s_nisp_node *f_nisp_clone_node(s_nisp *nisp, s_nisp_node *node) {
  s_nisp_node *result = NULL;
  if (node) {
    if ((result = f_nisp_generate_node(nisp, node->type))) {
      result->line_definition = node->line_definition;
      result->token_from_lexer = node->token_from_lexer;
      switch (node->type) {
        case e_nisp_node_native_symbol:
        case e_nisp_node_atom_symbol:
        case e_nisp_node_atom_token: {
          if (!node->token_from_lexer) {
            switch (node->value.token->type) {
              case e_token_type_word:
              case e_token_type_string: {
                result->value.token = f_tokens_new_token_char(node->value.token->token.token_char, node->value.token->type);
                break;
              }
              case e_token_type_value: {
                result->value.token = f_tokens_new_token_value(node->value.token->token.token_value);
                result->value.token->token.decimal_digits = node->value.token->token.decimal_digits;
                break;
              }
              case e_token_type_symbol: {
                result->value.token = f_tokens_new_token_symbol(node->value.token->token.token_symbol);
                break;
              }
              default: {}
            }
          } else
            result->value.token = node->value.token;
          break;
        }
        case e_nisp_node_list: {
          s_nisp_node *node_list_entry;
          d_list_foreach(&(node->value.list), node_list_entry, s_nisp_node)
            f_list_append(&(result->value.list), (s_list_node *)f_nisp_clone_node(nisp, node_list_entry), e_list_insert_tail);
          break;
        }
        case e_nisp_node_native_lambda: {
          s_nisp_node *node_list_entry;
          d_list_foreach(&(node->value.native_lambda.list), node_list_entry, s_nisp_node)
            f_list_append(&(result->value.list), (s_list_node *)f_nisp_clone_node(nisp, node_list_entry), e_list_insert_tail);
          result->value.native_lambda.routine = node->value.native_lambda.routine;
          break;
        }
        default: {}
      }
    }
  }
  return result;
}
s_nisp_node *f_nisp_generate_node(s_nisp *nisp, e_nisp_node_type type) {
  s_nisp_node_garbage_collector *garbage_entry;
  s_nisp_node *embedded_node = NULL;
  if ((garbage_entry = (s_nisp_node_garbage_collector *)d_malloc(sizeof(s_nisp_node_garbage_collector)))) {
    memset(garbage_entry, 0, sizeof(s_nisp_node_garbage_collector));
    embedded_node = &(garbage_entry->node);
    embedded_node->type = type;
    if (nisp)
      f_list_append(&(nisp->garbage_collector), (s_list_node *)garbage_entry, e_list_insert_tail);
  }
  return embedded_node;
}
s_nisp_node *f_nisp_generate_node_from_token(s_nisp *nisp, s_token *token) {
  s_nisp_node *result;
  if ((result = f_nisp_generate_node(nisp, ((token->type == e_token_type_word)||(token->type == e_token_type_symbol))?
    e_nisp_node_atom_symbol:e_nisp_node_atom_token))) {
    result->line_definition = token->line;
    result->value.token = token;
  }
  return result;
}
void f_nisp_append_native_lambda(s_nisp *nisp, const char *symbol, char *parameters[], l_nisp_native_lambda *routine) {
  s_nisp_node_environment *environment_entry = (s_nisp_node_environment *)f_dictionary_get_or_create(&(nisp->root_environment.symbols), symbol);
  if (environment_entry) {
    s_nisp_node *lambda_node;
    if ((lambda_node = f_nisp_generate_node(nisp, e_nisp_node_native_lambda))) {
      s_nisp_node *lambda_symbol_node;
      if ((lambda_symbol_node = f_nisp_generate_node_from_token(nisp, f_tokens_new_token_char("lambda", e_token_type_word)))) {
        s_nisp_node *parameters_list_node;
        if ((parameters_list_node = f_nisp_generate_node(nisp, e_nisp_node_list))) {
          size_t index_parameters = 0;
          while (parameters[index_parameters]) {
            s_nisp_node *new_parameter_symbol = f_nisp_generate_node_from_token(nisp, f_tokens_new_token_char(parameters[index_parameters], e_token_type_word));
            if (new_parameter_symbol)
              f_list_append(&(parameters_list_node->value.list), (s_list_node *)new_parameter_symbol, e_list_insert_tail);
            ++index_parameters;
          }
          f_list_append(&(lambda_node->value.list), (s_list_node *)lambda_symbol_node, e_list_insert_tail);
          f_list_append(&(lambda_node->value.list), (s_list_node *)parameters_list_node, e_list_insert_tail);
          lambda_node->value.native_lambda.routine = routine;
          environment_entry->value = lambda_node;
        }
      }
    }
  }
}
s_nisp_node *f_nisp_append_native_symbol(s_nisp *nisp, const char *symbol) {
  s_nisp_node *lambda_node = NULL;
  s_nisp_node_environment *environment_entry = (s_nisp_node_environment *)f_dictionary_get_or_create(&(nisp->root_environment.symbols), symbol);
  if (environment_entry) {
    if ((lambda_node = f_nisp_generate_node(nisp, e_nisp_node_native_symbol)))
      environment_entry->value = lambda_node;
  }
  return lambda_node;
}
static bool p_nisp_verify_abstract_syntax_tree_node(const s_nisp_node *node, const char format) {
  bool result = false;
  if ((strchr("stl?*", format)) && (node))
    if (((format == 's') && ((node->type == e_nisp_node_atom_symbol) || (node->type == e_nisp_node_native_symbol))) ||
      ((format == 't') && (node->type == e_nisp_node_atom_token)) ||
      ((format == 'S') && (node->type == e_nisp_node_atom_token) && (node->value.token->type == e_token_type_string)) ||
      ((format == 'D') && (node->type == e_nisp_node_atom_token) && (node->value.token->type == e_token_type_value)) ||
      ((format == 'l') && (node->type == e_nisp_node_list)) ||
      ((format == '?')))
      result = true;
  return result;
}
static bool p_nisp_verify_abstract_syntax_tree_through_list(s_nisp_node *current_list_node, const char *format) {
  bool result = true;
  if (format) {
    size_t index_format = 0;
    while ((result) && (current_list_node) && (format[index_format])) {
      if (format[index_format] == '*') {
        if (index_format > 0) {
          bool at_least_one_matches = false;
          while ((current_list_node) && (p_nisp_verify_abstract_syntax_tree_node(current_list_node, format[index_format - 1]))) {
            current_list_node = d_nisp_next(current_list_node);
            at_least_one_matches = true;
          }
          result = at_least_one_matches;
        } else
          result = false; /* the pattern starts with a *, and this is unacceptable */
      } else if (p_nisp_verify_abstract_syntax_tree_node(current_list_node, format[index_format]))
        current_list_node = d_nisp_next(current_list_node);
      ++index_format;
    }
    if ((current_list_node) || (format[index_format]))
      result = false;
  }
  return result;
}
static bool p_nisp_verify_abstract_syntax_tree(const s_nisp_node *root) {
  const struct {
    char *symbol, *format;
  } nisp_forms[] = {
    /* format syntax follows this approach:
     * - s: symbol
     * - S: a token of type string
     * - D: a token of type value
     * - t: a token
     * - l: a list
     * - ?: any kind
     * - *: the previous kind, repeated multiple times (at least once)
     */
    { "quote", "s?*" },
    { "lambda", "sl*" },
    { "define", "ss?" },
    { "set", "ss?" },
    { "begin", "sl*" },
    { "if", "sl??" },
    { NULL, NULL }
  };
  bool result = true;
  if ((root) && (root->type == e_nisp_node_list)) {
    s_nisp_node *current_node = (s_nisp_node *)root->value.list.head;
    if (current_node) {
      if (current_node->type == e_nisp_node_atom_symbol) {
        for (size_t index_symbol = 0; ((result) && (nisp_forms[index_symbol].symbol)); ++index_symbol)
          if (d_token_string_compare(current_node->value.token, nisp_forms[index_symbol].symbol))
            if (!((result = p_nisp_verify_abstract_syntax_tree_through_list(current_node, nisp_forms[index_symbol].format))))
              fprintf(stderr, "symbol <%s> doesn't respect the language syntax (line: %zu)\n",
                nisp_forms[index_symbol].symbol,
                current_node->line_definition);
        current_node = d_nisp_next(current_node);
      }
      while ((result) && (current_node)) {
        result = p_nisp_verify_abstract_syntax_tree(current_node);
        current_node = d_nisp_next(current_node);
      }
    }
  }
  return result;
}
static s_nisp_node *p_nisp_generate_abstract_syntax_tree(s_nisp *nisp, s_token **token) {
  s_nisp_node *result = NULL;
  if (*token)
    if ((result = f_nisp_generate_node(nisp, e_nisp_node_list))) {
      ssize_t ignore_line = -1;
      bool scope_termination = false;
      result->line_definition = (*token)->line;
      while ((!scope_termination) && (*token)) {
        s_token *current_token = *token;
        *token = (s_token *)((s_list_node *)(*token))->next;
        if (ignore_line != current_token->line) {
          if ((current_token->type == e_token_type_symbol) &&  (current_token->token.token_symbol == ';'))
            ignore_line = (ssize_t)current_token->line;
          else if ((current_token->type == e_token_type_symbol) &&  (current_token->token.token_symbol == ')'))
            scope_termination = true;
          else if ((current_token->type == e_token_type_symbol) &&  (current_token->token.token_symbol == '(')) {
            s_nisp_node *nisp_node = p_nisp_generate_abstract_syntax_tree(nisp, token);
            if (nisp_node)
              f_list_append(&(result->value.list), (s_list_node *)nisp_node, e_list_insert_tail);
          } else {
            s_nisp_node *nisp_node = f_nisp_generate_node_from_token(nisp, current_token);
            if (nisp_node) {
              nisp_node->token_from_lexer = 1; /* this token comes from the tokenizer,
                                                * therefore shouldn't be deleted in case the node isn't marked by the GC */
              f_list_append(&(result->value.list), (s_list_node *)nisp_node, e_list_insert_tail);
            }
          }
        }
      }
    }
  return result;
}
s_nisp_node_environment *f_nisp_lookup_environment_label(const char *symbol, s_nisp_environment *environment) {
  s_nisp_node_environment *result = NULL;
  if (environment)
    if (symbol)
      if (!(result = (s_nisp_node_environment *)f_dictionary_get_if_exists(&(environment->symbols), symbol)))
        result = f_nisp_lookup_environment_label(symbol, environment->parent);
  return result;
}
s_nisp_node_environment *f_nisp_lookup_environment_node(const s_nisp_node *symbol, s_nisp_environment *environment) {
  s_nisp_node_environment *result = NULL;
  if (environment)
    if ((symbol) && (symbol->type == e_nisp_node_atom_symbol) && (symbol->value.token->type == e_token_type_word))
      result = f_nisp_lookup_environment_label(symbol->value.token->token.token_char, environment);
  return result;
}
static s_nisp_node *p_nisp_evaluate(s_nisp *nisp, s_nisp_node *root, s_nisp_environment *environment);
static s_nisp_node *p_nisp_execute_lambda(s_nisp *nisp, const char *lambda_symbol, const s_nisp_node *lambda, s_nisp_node *head_parameters_list,
  s_nisp_environment *environment) {
  s_nisp_node *result = NULL;
  if ((lambda->type == e_nisp_node_lambda) || (lambda->type == e_nisp_node_native_lambda)) {
    s_nisp_node *current_parameter = head_parameters_list, *current_parameter_symbol = NULL,
      *lambda_parameters_symbol_list = d_nisp_next(lambda->value.list.head);
    size_t index_parameter = 0;
    bool invalid = false;
    s_nisp_environment deeper_environment = {
      .parent =  environment
    };
    f_dictionary_initialize(&(deeper_environment.symbols), sizeof(s_nisp_node_environment));
    if (lambda_parameters_symbol_list)
      current_parameter_symbol = (s_nisp_node *)lambda_parameters_symbol_list->value.list.head;
    while ((!invalid) && (current_parameter)) {
      s_nisp_node_environment *environment_entry_current_parameter = NULL;
      if (current_parameter_symbol) {
        if (current_parameter_symbol->type == e_nisp_node_atom_symbol) {
          environment_entry_current_parameter = (s_nisp_node_environment *)f_dictionary_get_or_create(&(deeper_environment.symbols),
            current_parameter_symbol->value.token->token.token_char);
          if (environment_entry_current_parameter)
            environment_entry_current_parameter->value = p_nisp_evaluate(nisp, current_parameter, deeper_environment.parent);
          current_parameter = d_nisp_next(current_parameter);
          current_parameter_symbol = d_nisp_next(current_parameter_symbol);
        } else {
          fprintf(stderr, "could not find a valid symbol in the parameters list on the definition of the lambda function <%s> (line: %zu)\n",
            lambda_symbol,current_parameter_symbol->line_definition);
          invalid = true;
        }
      } else {
        fprintf(stderr, "too many parameters passed to the lambda function <%s> (line: %zu)\n",
          lambda_symbol, head_parameters_list->line_definition);
        invalid = true;
      }
      ++index_parameter;
    }
    if (!current_parameter) {
      if (!current_parameter_symbol) {
        if (lambda->type == e_nisp_node_native_lambda)
          result = lambda->value.native_lambda.routine(nisp, &(deeper_environment));
        else {
          s_nisp_node *lambda_body_instruction = d_nisp_next(d_nisp_next(lambda->value.list.head));
          while (lambda_body_instruction) {
            result = p_nisp_evaluate(nisp, lambda_body_instruction, &deeper_environment);
            lambda_body_instruction = d_nisp_next(lambda_body_instruction);
          }
        }
      }
    } else
      fprintf(stderr, "too few parameters passed to the lambda function <%s> (line: %zu)\n",
        lambda_symbol, head_parameters_list->line_definition);
    f_dictionary_free(&(deeper_environment.symbols));
  }
  return result;
}
static s_nisp_node *p_nisp_evaluate_native_lambda(s_nisp_node *root, s_nisp_environment *environment) {
  root->type = e_nisp_node_lambda;
  return root;
}
static s_nisp_node *p_nisp_evaluate_native_lambda_define(s_nisp *nisp, s_nisp_node *root, s_nisp_environment *environment) {
  const s_nisp_node *environment_label = d_nisp_next(root->value.list.head);
  s_nisp_node_environment *environment_value = (s_nisp_node_environment *)f_dictionary_get_or_create(&(environment->symbols),
    environment_label->value.token->token.token_char);
  if (environment_value) {
    s_nisp_environment deeper_environment = {
      .parent =  environment
    };
    f_dictionary_initialize(&(deeper_environment.symbols), sizeof(s_nisp_node_environment));
    environment_value->value = p_nisp_evaluate(nisp, d_nisp_next(d_nisp_next(root->value.list.head)), &deeper_environment);
    f_dictionary_free(&(deeper_environment.symbols));
  }
  return root;
}
static s_nisp_node *p_nisp_evaluate_native_lambda_set(s_nisp *nisp, s_nisp_node *root, s_nisp_environment *environment) {
  const s_nisp_node *environment_label = d_nisp_next(root->value.list.head);
  s_nisp_node_environment *environment_node = f_nisp_lookup_environment_node(environment_label, environment);
  if (environment_node)
    environment_node->value = p_nisp_evaluate(nisp, d_nisp_next(d_nisp_next(root->value.list.head)), environment);
  else
    fprintf(stderr, "symbol <%s> not found (line: %zu)\n", environment_label->value.token->token.token_char,
      environment_label->line_definition);
  return root;
}
static s_nisp_node *p_nisp_evaluate_native_lambda_quote(s_nisp *nisp, s_nisp_node *root, s_nisp_environment *environment) {
  s_nisp_node *result = f_nisp_generate_node(nisp, e_nisp_node_list), *current_node = root;
  while (current_node) {
    f_list_append(&(result->value.list), (s_list_node *)f_nisp_clone_node(nisp, p_nisp_evaluate(nisp, current_node, environment)), e_list_insert_tail);
    current_node = d_nisp_next(current_node);
  }
  return result;
}
static s_nisp_node *p_nisp_evaluate_native_lambda_begin(s_nisp *nisp, s_nisp_node *root, s_nisp_environment *environment) {
  s_nisp_node *result = NULL, *current_node = d_nisp_next(root->value.list.head);
  while (current_node) {
    result = p_nisp_evaluate(nisp, current_node, environment);
    current_node = d_nisp_next(current_node);
  }
  return result; /* we're going to return the last element of a the list */
}
static s_nisp_node *p_nisp_evaluate_native_lambda_if(s_nisp *nisp, s_nisp_node *root, s_nisp_environment *environment) {
  s_nisp_node *result = NULL;
  if (root) {
    s_nisp_node *condition = d_nisp_next(root->value.list.head), *true_branch = d_nisp_next(condition),
      *false_branch = d_nisp_next(true_branch);
    if ((condition) && (true_branch)) {
      s_nisp_node *result_condition;
      if (((result_condition = p_nisp_evaluate(nisp, condition, environment)) != NULL) &&
        (nisp->false_symbol != result_condition) && (nisp->nil_symbol != result_condition))
        result = p_nisp_evaluate(nisp, true_branch, environment);
      else
        result = p_nisp_evaluate(nisp, false_branch, environment);
    }
  }
  return result;
}
static s_nisp_node *p_nisp_evaluate(s_nisp *nisp, s_nisp_node *root, s_nisp_environment *environment) {
  s_nisp_node *result = root;
  if (root) {
    switch (root->type) {
      case e_nisp_node_atom_token:
      case e_nisp_node_native_symbol: { /* basic case: returns the token itself */
        result = root;
        break;
      }
      case e_nisp_node_atom_symbol: { /* still a simple case scenario: we'll return the value of the symbol in the environment */
        s_nisp_node_environment *environment_node = f_nisp_lookup_environment_node(root, environment);
        if (environment_node)
          result = environment_node->value;
        break;
      }
      case e_nisp_node_list: {
        s_nisp_node *list_head = (s_nisp_node *)root->value.list.head;
        if (list_head) {
          s_nisp_node_environment *environment_node = NULL;
          if (list_head->type == e_nisp_node_atom_symbol) {
            if (d_token_string_compare(list_head->value.token, "quote"))
              result = p_nisp_evaluate_native_lambda_quote(nisp, d_nisp_next(list_head), environment);
            else if (d_token_string_compare(list_head->value.token, "lambda"))
              result = p_nisp_evaluate_native_lambda(root, environment);
            else if (d_token_string_compare(list_head->value.token, "define"))
              result = p_nisp_evaluate_native_lambda_define(nisp, root, environment);
            else if (d_token_string_compare(list_head->value.token, "set"))
              result = p_nisp_evaluate_native_lambda_set(nisp, root, environment);
            else if (d_token_string_compare(list_head->value.token, "begin"))
              result = p_nisp_evaluate_native_lambda_begin(nisp, root, environment);
            else if (d_token_string_compare(list_head->value.token, "if"))
              result = p_nisp_evaluate_native_lambda_if(nisp, root, environment);
            else if ((environment_node = f_nisp_lookup_environment_node(list_head, environment))) {
              if ((result = environment_node->value)) {
                if ((result->type == e_nisp_node_lambda) || (result->type == e_nisp_node_native_lambda))
                  result = p_nisp_execute_lambda(nisp, list_head->value.token->token.token_char, result, d_nisp_next(list_head), environment);
                else if (result->type == e_nisp_node_list)
                  fprintf(stderr, "symbol <%s> it is not a function (line: %zu)\n",
                    list_head->value.token->token.token_char,
                    list_head->line_definition);
              } else
                fprintf(stderr, "symbol <%s> is NULL (line: %zu)\n",
                  list_head->value.token->token.token_char,
                  list_head->line_definition);
            } else
              fprintf(stderr, "symbol <%s> not found (line: %zu)\n",
                list_head->value.token->token.token_char,
                list_head->line_definition);
          } else if ((result = p_nisp_evaluate(nisp, list_head, environment))) {
            if ((result->type == e_nisp_node_lambda) || (result->type == e_nisp_node_native_lambda))
              result = p_nisp_execute_lambda(nisp, "anonymous_lambda", result, d_nisp_next(list_head), environment);
            else
              fprintf(stderr, "unexpected non-lambda result for a non-empty list (we found a %s) (line: %zu)\n",
                m_nisp_node_types[result->type], result->line_definition);
          }
        }
        break;
      }
      default: { }
    }
  }
  return result;
}
void p_nisp_environment_load_standard_library(s_nisp *nisp) {
  nisp->nil_symbol = f_nisp_append_native_symbol(nisp, "NIL");
  nisp->true_symbol = f_nisp_append_native_symbol(nisp, "TRUE");
  nisp->false_symbol = f_nisp_append_native_symbol(nisp, "FALSE");
  f_nisp_append_native_lambda(nisp, "/", (char*[]){"a", "b", NULL}, f_nisp_standard_library_divide);
  f_nisp_append_native_lambda(nisp, "*", (char*[]){"a", "b", NULL}, f_nisp_standard_library_multiply);
  f_nisp_append_native_lambda(nisp, "+", (char*[]){"a", "b", NULL}, f_nisp_standard_library_sum);
  f_nisp_append_native_lambda(nisp, "-", (char*[]){"a", "b", NULL}, f_nisp_standard_library_subtract);
  f_nisp_append_native_lambda(nisp, "=", (char*[]){"a", "b", NULL}, f_nisp_standard_library_equal);
  f_nisp_append_native_lambda(nisp, "println", (char*[]){"quoted_list", NULL}, f_nisp_standard_library_println);
  f_nisp_append_native_lambda(nisp, "print", (char*[]){"quoted_list", NULL}, f_nisp_standard_library_print);
}
coremio_result f_nisp_environment_explode_buffer(const char *buffer, s_nisp *nisp) {
  coremio_result result;
  size_t line_accumulator = 0, character_accumulator = 0;
  memset(nisp, 0, sizeof(s_nisp));
  f_dictionary_initialize(&(nisp->root_environment.symbols), sizeof(s_nisp_node_environment));
  if ((result = f_tokens_explode_buffer(buffer, "();", NULL, " \n\r\t", &line_accumulator, &character_accumulator, &(nisp->tokens))) == NOICE) {
    s_token *current_token = (s_token *)nisp->tokens.head;
    p_nisp_environment_load_standard_library(nisp);
    nisp->root_code = p_nisp_generate_abstract_syntax_tree(nisp, &current_token);
    p_nisp_verify_abstract_syntax_tree(nisp->root_code);
  }
  return result;
}
coremio_result f_nisp_environment_explode_stream(int stream, s_nisp *nisp) {
  coremio_result result;
  memset(nisp, 0, sizeof(s_nisp));
  f_dictionary_initialize(&(nisp->root_environment.symbols), sizeof(s_nisp_node_environment));
  if ((result = f_tokens_explode_stream(stream, "();", NULL, " \n\r\t", &(nisp->tokens))) == NOICE) {
    s_token *current_token = (s_token *)nisp->tokens.head;
    p_nisp_environment_load_standard_library(nisp);
    nisp->root_code = p_nisp_generate_abstract_syntax_tree(nisp, &current_token);
    p_nisp_verify_abstract_syntax_tree(nisp->root_code);
  }
  return result;
}
coremio_result f_nisp_execute(s_nisp *nisp) {
  coremio_result result = NOICE;
  if ((nisp->root_code) && (nisp->root_code->type == e_nisp_node_list)) {
    s_nisp_node *current_instruction;
    d_list_foreach(&(nisp->root_code->value.list), current_instruction, s_nisp_node) {
      p_nisp_evaluate(nisp, current_instruction, &(nisp->root_environment));
      f_nisp_mark(nisp);
      f_nisp_sweep(nisp);
    }
    /* what should I do with the final value? */
    f_nisp_print_environment_plain(STDOUT_FILENO, &(nisp->root_environment), true);
  } else
    result = SHIT_INVALID_PARAMETERS;
  return result;
}
static void p_nisp_mark_nodes(s_nisp_node *root) {
  if (root) {
    root->mark = 1;
    if ((root->type == e_nisp_node_list) || (root->type == e_nisp_node_lambda) || (root->type == e_nisp_node_native_lambda)) {
      s_nisp_node *current_node;
      d_list_foreach(&(root->value.list), current_node, s_nisp_node)
        p_nisp_mark_nodes(current_node);
    }
  }
}
static void p_nisp_mark_environment_node_visiting(s_nisp_node_environment *node) {
  if (node)
    p_nisp_mark_nodes(node->value);
}
static void p_nisp_mark_environment(s_nisp_environment *environment) {
  if (environment) {
    f_dictionary_foreach(&(environment->symbols), (l_dictionary_node_visit)p_nisp_mark_environment_node_visiting, NULL);
    p_nisp_mark_environment(environment->parent);
  }
}
void f_nisp_mark(s_nisp *nisp) {
  s_nisp_node_garbage_collector *current_node;
  d_list_foreach(&(nisp->garbage_collector), current_node, s_nisp_node_garbage_collector)
    current_node->node.mark = 0;
  p_nisp_mark_nodes(nisp->root_code);
  p_nisp_mark_environment(&(nisp->root_environment));
}
void f_nisp_sweep(s_nisp *nisp) {
  s_nisp_node_garbage_collector *current_node = (s_nisp_node_garbage_collector *)nisp->garbage_collector.head, *next_node = NULL;
  while (current_node) {
    next_node = (s_nisp_node_garbage_collector *)((s_list_node *)current_node)->next;
    if (!current_node->node.mark) {
      if ((!current_node->node.token_from_lexer) &&
        ((current_node->node.type == e_nisp_node_atom_token) || (current_node->node.type == e_nisp_node_atom_symbol))) {
        f_tokens_free_token(current_node->node.value.token);
      }
      f_list_remove_from_owner((s_list_node *)current_node);
      d_free(current_node);
    }
    current_node = next_node;
  }
}
void f_nisp_free(s_nisp *nisp) {
  nisp->root_code = NULL;
  f_dictionary_free(&(nisp->root_environment.symbols));
  f_nisp_mark(nisp);
  f_nisp_sweep(nisp);
  f_tokens_free(&(nisp->tokens));
}
void p_nisp_print_nodes_plain(const int stream, const s_nisp_node *root, const size_t level) {
  if (root) {
    for (size_t index = 0; index < level; ++index)
      write(stream, "| ", 2);
    dprintf(stream, "| - [type: %s] ", m_nisp_node_types[root->type]);
    if ((root->type != e_nisp_node_list) &&
      (root->type != e_nisp_node_lambda) &&
      (root->type != e_nisp_node_native_lambda))
      f_tokens_print_detailed(stream, root->value.token);
    write(stream, "\n", 1);
    if ((root->type == e_nisp_node_list) ||
      (root->type == e_nisp_node_lambda) ||
      (root->type == e_nisp_node_native_lambda)) {
      s_nisp_node *current_node;
      d_list_foreach(&(root->value.list), current_node, s_nisp_node)
        p_nisp_print_nodes_plain(stream, current_node, (level + 1));
    }
  }
}
void f_nisp_print_nodes_plain(const int stream, const s_nisp_node *root) {
  if (root)
    p_nisp_print_nodes_plain(stream, root, 0);
}
struct s_nisp_graph_visit_payload {
  int stream;
  size_t level;
};
static void p_nisp_print_environment_node_plain_visiting(const s_nisp_node_environment *node, const struct s_nisp_graph_visit_payload *payload) {
  if (node) {
    for (size_t index = 0; index < payload->level; ++index)
      write(payload->stream, "| ", 2);
    dprintf(payload->stream, "| - label <%s> ", node->head.key);
    if (node->value) {
      dprintf(payload->stream, "[type: %s] ", m_nisp_node_types[node->value->type]);
      if (node->value->type == e_nisp_node_atom_token)
        f_tokens_print_detailed(payload->stream, node->value->value.token);
      else if (node->value->type == e_nisp_node_list)
        f_nisp_print_nodes_plain(payload->stream, node->value);
    } else
       dprintf(payload->stream, "NULL");
    write(payload->stream, "\n", 1);
  }
}
static void p_nisp_print_environment_plain(const int stream, const s_nisp_environment *environment, size_t *level, bool recursive) {
  if (environment) {
    if ((environment->parent) && (recursive))
      p_nisp_print_environment_plain(stream, environment->parent, level, recursive);
    f_dictionary_foreach(&(environment->symbols), (l_dictionary_node_visit)p_nisp_print_environment_node_plain_visiting,
      &(struct s_nisp_graph_visit_payload){stream, *level});
    ++(*level);
  }
}
void f_nisp_print_environment_plain(const int stream, const s_nisp_environment *environment, bool recursive) {
  size_t level = 0;
  p_nisp_print_environment_plain(stream, environment, &level, recursive);
}