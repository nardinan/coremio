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
#include "../include/coremio/tokens.h"
const char *m_tokens_types[] = {
  "undefined",
  "string",
  "word",
  "symbol",
  "value",
  "", // __STOOPIDITY RESERVED__
  "", // __STOOPIDITY RESERVED__
  ""  // __STOOPIDITY RESERVED__
};
static coremio_result p_tokens_append_characters(s_token *char_token, char *symbols_characters_table, char *starting_character,
  char *final_character, bool draft) {
  coremio_result result = NOICE;
  if (char_token->allocated) {
    bool remove_heading_character = false;
    size_t before_length = 0, current_size = 0;
    if (!draft) {
      if (((char_token->token.token_char) && (d_string_bootstrapper_character(symbols_characters_table, *(char_token->token.token_char)))) &&
          (d_string_bootstrapper_character(symbols_characters_table, *final_character))) {
        remove_heading_character = true;
        --final_character;
      } else if (((!char_token->token.token_char) && (d_string_bootstrapper_character(symbols_characters_table, *starting_character))) &&
                 (d_string_bootstrapper_character(symbols_characters_table, *final_character))) {
        ++starting_character;
        --final_character;
      }
    }
    if (char_token->token.token_char) {
      before_length = strlen(char_token->token.token_char);
      current_size = before_length;
      if (remove_heading_character) {
        memmove(char_token->token.token_char, char_token->token.token_char + 1, before_length);
        --before_length;
      }
    }
    if (final_character >= starting_character) {
      size_t additional_length = (final_character - starting_character) + 1;
      if (((before_length + additional_length) == current_size) ||
          ((char_token->token.token_char = (char *)d_realloc(char_token->token.token_char, (before_length + additional_length) + 1)))) {
        memset(char_token->token.token_char + before_length, 0, (additional_length + 1));
        strncpy(char_token->token.token_char + before_length, starting_character, additional_length);
      } else
        result = SHIT_NO_MEMORY;
    }
  } else {
    if (!draft) {
      if ((d_string_bootstrapper_character(symbols_characters_table, *starting_character)) &&
      (d_string_bootstrapper_character(symbols_characters_table, *final_character))) {
        ++starting_character;
        --final_character;
      }
    }
    if (final_character >= starting_character) {
      size_t length = (final_character - starting_character) + 1;
      if ((char_token->token.token_char = (char *)d_malloc(length + 1))) {
        memset(char_token->token.token_char, 0, length + 1);
        strncpy(char_token->token.token_char, starting_character, length);
      } else
        result = SHIT_NO_MEMORY;
      char_token->allocated = 1;
    } else
      char_token->token.token_char = NULL;
  }
  return result;
}
coremio_result f_tokens_explode_buffer(const char *buffer, char *symbols_characters_table, char *word_symbols_characters_table,
  char *ignorable_characters_table, size_t *line_accumulator, size_t *character_accumulator, s_list *tokens) {
  coremio_result result = NOICE;
  if (buffer) {
    char *current_character = (char *)buffer, *starting_character = NULL, last_character = 0;
    bool incomplete_pending_token = false, evaluate_string = true;
    s_token *current_token = NULL;
    if ((tokens->tail) && (!((s_token *)tokens->tail)->completed)) {
      current_token = (s_token *)tokens->tail;
      if ((current_token->type == e_token_type_string) && (current_token->token.token_char))
        last_character = current_token->token.token_char[strlen(current_token->token.token_char)];
      incomplete_pending_token = true;
    }
    if ((strchr(symbols_characters_table, '\'')) || (strchr(symbols_characters_table, '"')))
      evaluate_string = false;
    while ((result == NOICE) && (*current_character)) {
      bool jump_next_character = true;
      if (*current_character == '\n') {
        ++(*line_accumulator);
        *character_accumulator = 0;
      }
      if (current_token) {
        switch (current_token->type) {
          case e_token_type_string:
            if ((((current_token->token.token_char) && (*current_character == *(current_token->token.token_char))) ||
                 ((!current_token->token.token_char) && (starting_character) && (*current_character == *starting_character))) && (last_character != '\\')) {
              if (starting_character)
                result = p_tokens_append_characters(current_token, symbols_characters_table, starting_character, current_character, false);
              if ((incomplete_pending_token) && (current_token->token.token_char) &&
              (d_string_bootstrapper_character(symbols_characters_table, *(current_token->token.token_char))))
                memmove(current_token->token.token_char, (current_token->token.token_char + 1), strlen(current_token->token.token_char));
              current_token->completed = 1;
            } else if (!starting_character)
              starting_character = current_character;
            break;
          case e_token_type_word:
            if (((!word_symbols_characters_table) || (!strchr(word_symbols_characters_table, *current_character))) &&
                (((symbols_characters_table) && (strchr(symbols_characters_table, *current_character))) ||
                 ((ignorable_characters_table) && (strchr(ignorable_characters_table, *current_character))))) {
              if ((starting_character) && (current_character > 0))
                result = p_tokens_append_characters(current_token, symbols_characters_table, starting_character, (current_character - 1), false);
              current_token->completed = 1;
              jump_next_character = false;
            } else if (!starting_character)
              starting_character = current_character;
            break;
          case e_token_type_value:
            if ((*current_character == '.') && (!current_token->token.decimal_digits))
              ++(current_token->token.decimal_digits);
            else if (isdigit(*current_character)) {
              double change_sign = 1.f;
              if (current_token->token.token_value < 0)
                change_sign = -1.f;
              current_token->token.token_value = (current_token->token.token_value * 10.0f) + ((double)(*current_character - '0') * change_sign);
              if (current_token->token.decimal_digits > 0)
                ++(current_token->token.decimal_digits);
            } else {
              if (current_token->token.decimal_digits > 1)
                current_token->token.token_value /= pow(10.0f, (current_token->token.decimal_digits - 1));
              current_token->completed = 1;
              jump_next_character = false;
            }
            break;
          default:
            break;
        }
      } else if ((!ignorable_characters_table) || (!strchr(ignorable_characters_table, *current_character))) {
        incomplete_pending_token = false;
        if ((current_token = (s_token *)d_malloc(sizeof(s_token)))) {
          memset(current_token, 0, sizeof(s_token));
          current_token->line = *line_accumulator;
          current_token->character = *character_accumulator;
          if (d_string_bootstrapper_character(symbols_characters_table, *current_character)) {
            current_token->type = e_token_type_string;
            starting_character = current_character;
          } else if ((isdigit(*current_character)) ||
              ((d_value_bootstrapper_character(symbols_characters_table, *current_character)) && (isdigit(*(current_character + 1))))) {
            double change_sign = 1.f;
            current_token->type = e_token_type_value;
            if (d_value_bootstrapper_character(symbols_characters_table, *current_character)) {
              if (*current_character == '-')
                change_sign = -1.f;
              ++(*character_accumulator);
              ++current_character;
            }
            current_token->token.token_value = ((double)(*current_character - '0')) * change_sign;
          } else if (strchr(symbols_characters_table, *current_character)) {
            current_token->type = e_token_type_symbol;
            current_token->token.token_symbol = *current_character;
            current_token->completed = 1;
          } else {
            current_token->type = e_token_type_word;
            starting_character = current_character;
          }
          f_list_append(tokens, (s_list_node *)current_token, e_list_insert_tail);
        } else
          result = SHIT_NO_MEMORY;
      }
      if ((current_token) && (current_token->completed))
        current_token = NULL;
      if (jump_next_character) {
        last_character = *current_character;
        ++(*character_accumulator);
        ++current_character;
      }
    }
    if (result == NOICE)
      if ((current_token) && (!current_token->completed)) {
        /* we need to push into the stack every string/word ongoing */
        if (((current_token->type == e_token_type_word) || (current_token->type == e_token_type_string)) && (current_character > buffer))
          result = p_tokens_append_characters(current_token, symbols_characters_table, starting_character, (current_character - 1), true);
      }
  }
  return result;
}
coremio_result f_tokens_explode_stream(int stream, char *symbols_characters_table, char *word_symbols_characters_table,
  char *ignorable_characters_table, s_list *tokens) {
  coremio_result result = NOICE;
  char buffer[d_string_buffer_size];
  size_t bytes, line = 0, character = 0;
  while ((result == NOICE) && ((bytes = read(stream, buffer, (d_string_buffer_size - 1))) > 0)) {
    buffer[bytes] = 0;
    result = f_tokens_explode_buffer(buffer, symbols_characters_table, word_symbols_characters_table, ignorable_characters_table, &line, &character, tokens);
  }
  return result;
}
void f_tokens_free_token(s_token *token) {
  if (token) {
    if ((token->allocated) && (token->token.token_char))
      d_free(token->token.token_char);
    d_free(token);
  }
}
void f_tokens_free(s_list *tokens) {
  struct s_token *current_token;
  while ((current_token = (s_token *)tokens->head)) {
    f_list_remove(tokens, tokens->head);
    f_tokens_free_token(current_token);
  }
}
void f_tokens_print_detailed(int stream, s_token *token) {
  if (token) {
    switch(token->type) {
      case e_token_type_string:
      case e_token_type_word:
        dprintf(stream, "{(%s %lu@%lu) '%s'}", m_tokens_types[token->type], token->line, token->character,
          token->token.token_char);
        break;
      case e_token_type_symbol:
        dprintf(stream, "{(%s %lu@%lu) '%c'}", m_tokens_types[token->type], token->line, token->character,
          token->token.token_symbol);
        break;
      case e_token_type_value:
        dprintf(stream, "{(%s %lu@%lu) %.04f}", m_tokens_types[token->type], token->line, token->character,
          token->token.token_value);
        break;
      default:
        dprintf(stream, "{unknown token}");
    }
  } else
    dprintf(stream, "NULL");
}
void f_tokens_print_plain(int stream, s_token *token) {
  if (token)
    switch(token->type) {
      case e_token_type_string:
      case e_token_type_word:
        dprintf(stream, "%s", token->token.token_char);
        break;
      case e_token_type_symbol:
        dprintf(stream, "%c", token->token.token_symbol);
        break;
      case e_token_type_value:
        dprintf(stream, "%.04f", token->token.token_value);
      default:
        break;
    }
}
s_token *f_tokens_new_token_char(const char *value, e_token_types type) {
  s_token *result = NULL;
  if ((type == e_token_type_string) || (type == e_token_type_word))
    if ((result = (s_token *)d_malloc(sizeof(s_token)))) {
      size_t length = strlen(value);
      memset(result, 0, sizeof(s_token));
      result->allocated = 1;
      result->completed = 1;
      result->type = type;
      if ((result->token.token_char = (char *)d_malloc(length + 1))) {
        strcpy(result->token.token_char, value);
        result->token.token_char[length] = 0;
      } else {
        d_free(result);
        result = NULL;
      }
    }
  return result;
}
s_token *f_tokens_new_token_value(double value) {
  s_token *result = NULL;
  if ((result = (s_token *)d_malloc(sizeof(s_token)))) {
    memset(result, 0, sizeof(s_token));
    result->completed = 1;
    result->type = e_token_type_value;
    result->token.token_value = value;
  }
  return result;
}
s_token *f_tokens_new_token_symbol(char value) {
  s_token *result = NULL;
  if ((result = (s_token *)d_malloc(sizeof(s_token)))) {
    memset(result, 0, sizeof(s_token));
    result->allocated = 1;
    result->completed = 1;
    result->type = e_token_type_symbol;
    result->token.token_symbol = value;
  }
  return result;
}