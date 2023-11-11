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
#ifndef COREMIO_TOKENS_H
#define COREMIO_TOKENS_H
#include <stdio.h>
#include <stdbool.h>
#include <ctype.h>
#include <math.h>
#include "list.h"
#include "result.h"
#include "local.string.h"
#define d_string_bootstrapper_character(symbols,c) (((!strchr(symbols,'\''))&&((c)=='\''))||((!strchr(symbols,'"'))&&((c)=='"')))
#define d_value_bootstrapper_character(symbols,c) (((!strchr(symbols,'+'))&&((c)=='+'))||((!strchr(symbols,'-'))&&((c)=='-')))
#define d_token_string_compare(t,s) ((((t)->type==e_token_type_word)||((t)->type==e_token_type_string))&&\
    ((t)->token.token_char)&&(strcmp((t)->token.token_char,s)==0))
#define d_token_numeric_compare(t,n) (((t)->type==e_token_type_value)&&((t)->token.token_value==(n)))
typedef enum e_token_types {
  e_token_type_undefined = 0,
  e_token_type_string,
  e_token_type_word,
  e_token_type_symbol,
  e_token_type_value
} e_token_types;
extern const char *m_tokens_types[];
typedef struct s_token {
  s_list_node head;
  e_token_types type;
  unsigned char completed:1, allocated:1;
  size_t line, character;
  union {
    char *token_char, token_symbol;
    struct {
      double token_value;
      unsigned short int decimal_digits;
    };
  } token;
} s_token;
extern coremio_result f_tokens_explode_buffer(const char *buffer, const char *symbols_characters_table, const char *word_symbols_characters_table,
  const char *ignorable_characters_table, size_t *line_accumulator, size_t *character_accumulator, s_list *tokens);
extern coremio_result f_tokens_explode_stream(int stream, const char *symbols_characters_table, const char *word_symbols_characters_table,
  const char *ignorable_characters_table, s_list *tokens);
extern void f_tokens_free_token(s_token *token);
extern void f_tokens_free(s_list *tokens);
extern void f_tokens_print_detailed(int stream, const s_token *token);
extern void f_tokens_print_plain(int stream, const s_token *token);
extern s_token *f_tokens_new_token_char(const char *value, e_token_types type);
extern s_token *f_tokens_new_token_value(double value);
extern s_token *f_tokens_new_token_symbol(char value);
#endif //COREMIO_TOKENS_H
