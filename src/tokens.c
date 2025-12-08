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
static coremio_result p_tokens_append_characters(t_token* string_token, const char* symbols_characters_table, const char* starting_character,
  const char* final_character) {
  coremio_result result = NOICE;
  if (final_character >= starting_character) {
    size_t additional_length = (final_character - starting_character) + 1;
    if (d_boxed_nan_get_signature(*string_token) == d_boxed_nan_embedded_string_signature) {
      /* it is an embedded string; Do we still have space to put the residual string or we have to demote the embedded string to a pointer? */
      char embedded_string[d_boxed_nan_available_bytes] = {0};
      size_t length, new_length;
      f_boxed_nan_get_embedded_string(*string_token, embedded_string);
      if ((new_length = ((length = strlen(embedded_string)) + additional_length)) < d_boxed_nan_available_bytes) {
        if (final_character >= starting_character) {
          strncpy(embedded_string + length, starting_character, additional_length);
          embedded_string[new_length] = 0;
        }
        *string_token = f_boxed_nan_embedded_string(embedded_string, 0);
      } else {
        char* pointer_string;
        if ((pointer_string = d_malloc(new_length + 1))) {
          strncpy(pointer_string, embedded_string, length);
          strncpy(pointer_string + length, starting_character, additional_length);
          pointer_string[new_length] = 0;
          *string_token = f_boxed_nan_pointer_string(pointer_string);
        } else
          result = SHIT_NO_MEMORY;
      }
    } else if ((d_boxed_nan_get_signature(*string_token) == d_boxed_nan_pointer_string_signature) ||
      (d_boxed_nan_get_signature(*string_token) == d_boxed_nan_pointer_quoted_string_signature)) {
      /* it is a pointer string; Do we still have space to put the residual string or we have to demote the pointer string to an embedded string? */
      char* pointer_string = d_boxed_nan_get_pointer(*string_token);
      const size_t length = ((pointer_string) ? strlen(pointer_string) : 0), new_length = length + additional_length;
      if ((pointer_string = (char*)d_realloc(pointer_string, new_length + 1))) {
        strncpy(pointer_string + length, starting_character, additional_length);
        pointer_string[new_length] = 0;
        if (d_boxed_nan_get_signature(*string_token) == d_boxed_nan_pointer_quoted_string_signature) {
          if ((pointer_string[new_length - 2] != '\\') && (d_token_string_bootstrapper_character(symbols_characters_table, pointer_string[new_length - 1])) &&
            (pointer_string[0] == pointer_string[new_length - 1])) {
            /* we're going to clean the starting characters that surround the string */
            memmove(pointer_string, (pointer_string + 1), (new_length - 1));
            pointer_string[new_length - 2] = 0;
          }
          *string_token = f_boxed_nan_pointer_custom(pointer_string);
        } else
          *string_token = f_boxed_nan_pointer_string(pointer_string);
      } else
        result = SHIT_NO_MEMORY;
    } else
      result = SHIT_INVALID_PARAMETERS;
  }
  return result;
}
coremio_result f_tokens_explode_buffer(const char* buffer, const char* symbols_characters_table, const char* word_symbols_characters_table,
  const char* ignorable_characters_table, size_t* line_accumulator, size_t *line_breaks_accumulator, size_t* character_accumulator, size_t* token_index,
  bool* last_token_incomplete, t_token** link_tokens) {
  coremio_result result = NOICE;
  if (buffer) {
    t_token* tokens = *link_tokens;
    const char *current_character = (char*)buffer, *starting_character = NULL;
    char last_character = 0;
    bool jump_next_character = true;
    if ((*token_index > 0) && (*last_token_incomplete)) {
      /* there's still a pending token, at the end of the array, so we need to complete it */
      const size_t previous_token_index = (*token_index - 1);
      const int current_token_signature = d_boxed_nan_get_signature(tokens[previous_token_index]);
      if ((current_token_signature == d_boxed_nan_pointer_string_signature) || (current_token_signature == d_boxed_nan_pointer_quoted_string_signature)) {
        const char* pointer_string = d_boxed_nan_get_pointer(tokens[previous_token_index]);
        const size_t length = (pointer_string) ? strlen(pointer_string) : 0;
        if (length > 0)
          last_character = ((char*)d_boxed_nan_get_pointer(tokens[previous_token_index]))[length - 1];
      } else if (current_token_signature == d_boxed_nan_embedded_string_signature) {
        char embedded_string[d_boxed_nan_available_bytes] = {0};
        size_t length = 0;
        f_boxed_nan_get_embedded_string(tokens[previous_token_index], embedded_string);
        if ((length = strlen(embedded_string)) > 0)
          last_character = embedded_string[length - 1];
      }
    }
    while ((result == NOICE) && (*current_character)) {
      if (jump_next_character) {
        if (*current_character == '\n') {
          ++(*line_accumulator);
          ++(*line_breaks_accumulator);
          *character_accumulator = 0;
        }
      }
      jump_next_character = true;
      if ((*token_index > 0) && (*last_token_incomplete)) {
        const size_t previous_token_index = (*token_index - 1);
        switch (d_boxed_nan_get_signature(tokens[previous_token_index])) {
          case d_boxed_nan_pointer_string_signature:
          case d_boxed_nan_embedded_string_signature: {
            if (((!word_symbols_characters_table) || (!strchr(word_symbols_characters_table, *current_character))) &&
              (((symbols_characters_table) && (strchr(symbols_characters_table, *current_character))) ||
                ((ignorable_characters_table) && (strchr(ignorable_characters_table, *current_character))))) {
              /* in this case, we have encountered a character that stops the word processing; This means that we have to dump what we have read so far, and
               * create jump on the next token. The current_character shall be processed by the next token readout */
              if ((starting_character) && (current_character > 0))
                result = p_tokens_append_characters(&(tokens[previous_token_index]), symbols_characters_table, starting_character, (current_character - 1));
              *last_token_incomplete = false;
              jump_next_character = false;
            } else if (!starting_character)
              starting_character = current_character;
            break;
          }
          case d_boxed_nan_pointer_quoted_string_signature: {
            char* pointer_string = d_boxed_nan_get_pointer(tokens[previous_token_index]);
            if ((((pointer_string) && (*current_character == *pointer_string)) ||
              ((!pointer_string) && (starting_character) && (*current_character == *starting_character))) && (last_character != '\\')) {
              /* in this case, we know that either the current character is equal to the first one (as we're on a quoted string signature, so the first
               * character is a character in the d_string_bootstrap_character set. If the string starts with " it should end up with ", if it starts with '
               * it should ends up with ') or - in case this was a research not yet dumped into a token - that the current character is
               * equal to the first one inside "starting_character" and that the character before this one, isn't a '\' character */
              if (!starting_character)
                starting_character = current_character;
              result = p_tokens_append_characters(&(tokens[previous_token_index]), symbols_characters_table, starting_character, current_character);
              pointer_string = d_boxed_nan_get_pointer(tokens[previous_token_index]);
              *last_token_incomplete = false;
            } else if (!starting_character)
              starting_character = current_character;
            break;
          }
          case d_boxed_nan_int_signature: {
            /* A number starts as an integer, every single time. As soon as we reach its decimal component, we promote it to a double */
            if (isdigit(*current_character))
              tokens[previous_token_index] = f_boxed_nan_int((d_boxed_nan_get_int(tokens[previous_token_index]) * 10) + (*current_character - '0'));
            else if (*current_character == '.')
              tokens[previous_token_index] = (double)d_boxed_nan_get_int(tokens[previous_token_index]);
            else {
              *last_token_incomplete = false;
              jump_next_character = false;
            }
            break;
          }
          case d_boxed_nan_nan_signature:
          default: {
            if (!isdigit(*current_character)) {
              if (starting_character)
                tokens[previous_token_index] /= pow(10.0f, (double)(current_character - starting_character));
              *last_token_incomplete = false;
              jump_next_character = false;
            } else {
              tokens[previous_token_index] = ((tokens[previous_token_index] * 10) + (*current_character - '0'));
              if (!starting_character)
                starting_character = current_character;
            }
            break;
          }
        }
      } else if ((!ignorable_characters_table) || (!strchr(ignorable_characters_table, *current_character))) {
        *last_token_incomplete = false;
        starting_character = NULL;
        if ((*line_breaks_accumulator) > 0) {
          for (size_t index_line_break = 0; index_line_break < (*line_breaks_accumulator); ++index_line_break)
            if ((*link_tokens = f_array_validate_access(*link_tokens, *token_index))) {
              tokens = *link_tokens;
              tokens[*token_index] = ((u_boxed_nan_container){ .integer_value = ((((int64_t)d_boxed_nan_new_line_signature) << 48) | 0) }).double_value;
              ++(*token_index);
            }
          (*line_breaks_accumulator) = 0;
        }
        if ((*link_tokens = f_array_validate_access(*link_tokens, *token_index))) {
          tokens = *link_tokens;
          if (d_token_string_bootstrapper_character(symbols_characters_table, *current_character)) {
            tokens[*token_index] = f_boxed_nan_pointer_custom(NULL);
            starting_character = current_character;
            *last_token_incomplete = true;
          } else if ((isdigit(*current_character)) ||
            ((d_token_value_bootstrapper_character(symbols_characters_table, *current_character)) && (isdigit(*(current_character + 1))))) {
            int change_sign = 1;
            if (d_token_value_bootstrapper_character(symbols_characters_table, *current_character)) {
              if (*current_character == '-')
                change_sign = -1;
              /* now, as we need a first digit, we're going to move the cursor forward */
              ++(*character_accumulator);
              ++current_character;
            }
            tokens[*token_index] = f_boxed_nan_int((*current_character - '0') * change_sign);
            *last_token_incomplete = true;
          } else if ((symbols_characters_table) && (strchr(symbols_characters_table, *current_character)))
            tokens[*token_index] = f_boxed_nan_embedded_string((char[]){*current_character, 0, d_boxed_nan_special_character_symbol}, 3);
          else {
            tokens[*token_index] = f_boxed_nan_embedded_string((char[]){0}, 0);
            starting_character = current_character;
            *last_token_incomplete = true;
          }
          ++(*token_index);
        } else
          result = SHIT_NO_MEMORY;
      }
      if (jump_next_character) {
        last_character = *current_character;
        ++(*character_accumulator);
        ++current_character;
      }
    }
    if (result == NOICE)
      if (*last_token_incomplete) {
        /* we need to push into the stack every string/word ongoing */
        const size_t previous_token_index = (*token_index - 1);
        if (((d_boxed_nan_get_signature(tokens[previous_token_index]) == d_boxed_nan_pointer_string_signature) ||
          (d_boxed_nan_get_signature(tokens[previous_token_index]) == d_boxed_nan_embedded_string_signature) ||
          (d_boxed_nan_get_signature(tokens[previous_token_index]) == d_boxed_nan_pointer_quoted_string_signature)) && (starting_character) &&
          (current_character >= starting_character))
          result = p_tokens_append_characters(&(tokens[previous_token_index]), symbols_characters_table, starting_character, (current_character - 1));
      }
  }
  return result;
}
coremio_result f_tokens_explode_stream(const int stream, const char* symbols_characters_table, const char* word_symbols_characters_table,
  const char* ignorable_characters_table, t_token** tokens) {
  coremio_result result = NOICE;
  char buffer[d_string_buffer_size];
  size_t bytes, line = 0, line_breaks = 0, character = 0, index = 0;
  bool last_token_incomplete = false;
  while ((result == NOICE) && ((bytes = read(stream, buffer, (d_string_buffer_size - 1))) > 0)) {
    buffer[bytes] = 0;
    result = f_tokens_explode_buffer(buffer, symbols_characters_table, word_symbols_characters_table, ignorable_characters_table, &line, &line_breaks,
      &character, &index, &last_token_incomplete, tokens);
  }
  return result;
}
void f_tokens_free_token_content(const t_token token) {
  if (token)
    if (d_boxed_nan_get_signature(token) == d_boxed_nan_pointer_string_signature)
      d_free(d_boxed_nan_get_pointer(token));
}
void f_tokens_free(t_token* tokens) {
  if (tokens) {
    for (size_t index_entry = 0; index_entry < d_array_size(tokens); ++index_entry)
      if ((d_boxed_nan_get_signature(tokens[index_entry]) == d_boxed_nan_pointer_string_signature) ||
        (d_boxed_nan_get_signature(tokens[index_entry]) == d_boxed_nan_pointer_quoted_string_signature))
        d_free(d_boxed_nan_get_pointer(tokens[index_entry]));
    f_array_free(tokens);
  }
}
void f_tokens_print_detailed(const int stream, const t_token token) {
  switch (d_boxed_nan_get_signature(token)) {
    case d_boxed_nan_new_line_signature: {
      dprintf(stream, "{ new line }");
      break;
    }
    case d_boxed_nan_pointer_string_signature: {
      dprintf(stream, "{(string, word) '%s'}", (char*)d_boxed_nan_get_pointer(token));
      break;
    }
    case d_boxed_nan_pointer_quoted_string_signature: {
      dprintf(stream, "{(string, quoted) '%s'}", (char*)d_boxed_nan_get_pointer(token));
      break;
    }
    case d_boxed_nan_embedded_string_signature: {
      if (d_token_is_symbol(token))
        dprintf(stream, "{(string, symbol, embedded) '%s'}", (char *)&token);
      else
        dprintf(stream, "{(string, word, embedded) '%s'}", (char *)&token);
      break;
    }
    case d_boxed_nan_int_signature: {
      dprintf(stream, "{(int) %d}", (int)d_boxed_nan_get_int(token));
      break;
    }
    case d_boxed_nan_bool_signature: {
      dprintf(stream, "{(bool) %s}", (d_boxed_nan_get_boolean(token) ? "true" : "false"));
      break;
    }
    case d_boxed_nan_nan_signature:
    default: {
      dprintf(stream, "{(double) %f}", token);
      break;
    }
  }
}
void f_tokens_print_plain(const int stream, const t_token token) {
  switch (d_boxed_nan_get_signature(token)) {
    case d_boxed_nan_new_line_signature: {
      dprintf(stream, "{ new line }");
      break;
    }
    case d_boxed_nan_pointer_string_signature:
    case d_boxed_nan_pointer_quoted_string_signature: {
      dprintf(stream, "%s", (char*)d_boxed_nan_get_pointer(token));
      break;
    }
    case d_boxed_nan_embedded_string_signature: {
      dprintf(stream, "%s", (char *)&(token));
      break;
    }
    case d_boxed_nan_int_signature: {
      dprintf(stream, "%d", (int)d_boxed_nan_get_int(token));
      break;
    }
    case d_boxed_nan_bool_signature: {
      dprintf(stream, "%s", (d_boxed_nan_get_boolean(token) ? "true" : "false"));
      break;
    }
    case d_boxed_nan_nan_signature: default: {
      dprintf(stream, "%f", token);
      break;
    }
  }
}
t_token f_tokens_new_token_char(const char* value, bool quoted) {
  t_token result = NAN;
  size_t length = strlen(value);
  if ((length >= d_boxed_nan_available_bytes) || (quoted)) {
    char *string_buffer;
    if ((string_buffer = d_malloc(length + 1))) {
      strncpy(string_buffer, value, length);
      string_buffer[length] = 0;
      if (quoted)
        result = f_boxed_nan_pointer_custom(string_buffer);
      else
        result = f_boxed_nan_pointer_string(string_buffer);
    }
  } else
    result = f_boxed_nan_embedded_string(value, length);
  return result;
}
t_token f_tokens_new_token_symbol(const char value) {
  return f_boxed_nan_embedded_string((char[]){value, 0, d_boxed_nan_special_character_symbol}, 3);
}
t_token f_tokens_new_token_double(const double value) {
  return value;
}
t_token f_tokens_new_token_int(const int value) {
  return f_boxed_nan_int(value);
}
extern t_token f_tokens_new_token_bool(const bool value) {
  return f_boxed_nan_boolean(value);
}
bool f_tokens_compare(const t_token token_a, const t_token token_b) {
  bool result = true;
  if (token_a != token_b) {
    result = false;
    if ((d_boxed_nan_get_signature(token_a) == d_boxed_nan_get_signature(token_b)) &&
      ((d_boxed_nan_get_signature(token_a) == d_boxed_nan_pointer_string_signature) ||
        (d_boxed_nan_get_signature(token_a) == d_boxed_nan_pointer_quoted_string_signature)))
      if (strcmp(d_boxed_nan_get_pointer(token_a), d_boxed_nan_get_pointer(token_b)) == 0)
        result = true;
  }
  return result;
}
bool f_tokens_compare_string(const t_token token, const char* entry) {
  bool result = false;
  char embedded_string[d_boxed_nan_available_bytes] = {0};
  switch (d_boxed_nan_get_signature(token)) {
    case d_boxed_nan_pointer_custom_signature:
    case d_boxed_nan_pointer_string_signature: {
      if (strcmp(d_boxed_nan_get_pointer(token), entry) == 0)
        result = true;
      break;
    }
    case d_boxed_nan_embedded_string_signature: {
      f_boxed_nan_get_embedded_string(token, embedded_string);
      if (strcmp(embedded_string, entry) == 0)
        result = true;
    }
    default: {
    }
  }
  return result;
}