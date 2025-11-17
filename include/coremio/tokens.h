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
#include <math.h>
#include "array.h"
#include "boxed_nan.h"
#include "local.string.h"
#define d_boxed_nan_pointer_quoted_string_signature 0x7FFD
#define d_boxed_nan_new_line_signature 0x7FFE
#define d_boxed_nan_special_character_symbol 0xBA
#define d_token_is_new_line(t) ((d_boxed_nan_get_signature(t) == d_boxed_nan_new_line_signature))
#define d_token_is_symbol(t) ((d_boxed_nan_get_signature(t) == d_boxed_nan_embedded_string_signature) &&\
  (((((u_boxed_nan_container){.double_value=(t)}).integer_value >> 8) & 0xFF) == 0x00) &&\
  (((((u_boxed_nan_container){.double_value=(t)}).integer_value >> 16) & 0xFF) == d_boxed_nan_special_character_symbol))
#define d_token_is_given_symbol(t,c) ((d_token_is_symbol(t)) && ((*((char *)&(t)) == (c))))
#define d_token_is_string(t) ((d_boxed_nan_get_signature(t) == d_boxed_nan_embedded_string_signature) ||\
  (d_boxed_nan_get_signature(t) == d_boxed_nan_pointer_string_signature) ||\
  (d_boxed_nan_get_signature(t) == d_boxed_nan_pointer_quoted_string_signature))
#define d_token_is_value(t) ((d_boxed_nan_get_signature(t) == d_boxed_nan_int_signature) ||\
  (d_boxed_nan_get_signature(t) == d_boxed_nan_nan_signature) ||\
  (!d_boxed_nan_is_boxed_nan(t)))
#define d_token_string_bootstrapper_character(symbols,c) (((!strchr(symbols, '\'')) && ((c) == '\'')) || ((!strchr(symbols, '"')) && ((c) == '"')))
#define d_token_value_bootstrapper_character(symbols,c) (((!strchr(symbols, '+')) && ((c) == '+')) || ((!strchr(symbols, '-')) && ((c) == '-')))
typedef double t_token;
extern coremio_result f_tokens_explode_buffer(const char* buffer, const char* symbols_characters_table, const char* word_symbols_characters_table,
  const char* ignorable_characters_table, size_t* line_accumulator, size_t *line_breaks_accumulator, size_t* character_accumulator, size_t* token_index,
  bool* last_token_incomplete, t_token** tokens);
extern coremio_result f_tokens_explode_stream(int stream, const char* symbols_characters_table, const char* word_symbols_characters_table,
  const char* ignorable_characters_table, t_token** tokens);
extern void f_tokens_free_token_content(t_token token);
extern void f_tokens_free(t_token* tokens);
extern void f_tokens_print_detailed(int stream, t_token token);
extern void f_tokens_print_plain(int stream, t_token token);
extern t_token f_tokens_new_token_char(const char* value, bool quoted);
extern t_token f_tokens_new_token_symbol(char value);
extern t_token f_tokens_new_token_double(double value);
extern t_token f_tokens_new_token_int(int value);
extern t_token f_tokens_new_token_bool(bool value);
extern bool f_tokens_compare(t_token token_a, t_token token_b);
extern bool f_tokens_compare_string(t_token token, const char* entry);
#endif //COREMIO_TOKENS_H
