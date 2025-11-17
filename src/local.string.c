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
#include "../include/coremio/local.string.h"
char *f_string_trim(char *string) {
  if (string) {
    const size_t length = strlen(string);
    const char *begin = string;
    char *final = (string + length) - 1;
    while ((d_space_character(*begin)) && (final >= begin))
      ++begin;
    while (((d_space_character(*final)) || (d_final_character(*final))) && (final >= begin)) {
      *final = 0;
      --final;
    }
    if (begin > string)
      memmove(string, begin, strlen(begin) + 1);
  }
  return string;
}
char *f_string_format(char *buffer, size_t *computed_size, const size_t size, const char *symbols, t_string_formatter functions[], char *format, ...) {
  va_list parameters;
  va_start(parameters, format);
  f_string_format_args(buffer, computed_size, size, symbols, functions, format, parameters);
  va_end(parameters);
  return buffer;
}
static char *p_string_format_skip(char *buffer, const char *symbols) {
  while (strchr("#0-+ '", *buffer))
    buffer++;
  while (isdigit(*buffer))
    buffer++;
  if (*buffer == '.') {
    buffer++;
    if (isdigit(*buffer))
      while (isdigit(*buffer))
        buffer++;
    else
      buffer = NULL;
  }
  if (buffer) {
    while (strchr("hljztLq", *buffer))
      buffer++;
    if ((strchr("diouXxfFeEgGaAcsb%", *buffer)) || (strchr(symbols, *buffer)))
      buffer++;
    else
      buffer = NULL;
  }
  return buffer;
}
char *f_string_format_args(char *buffer, size_t *computed_size, const size_t size, const char *symbols, t_string_formatter functions[], char *format,
  va_list parameters) {
  char *target = buffer, *pointer = format, *next, *last, *tail;
  size_t dimension, remaining = 0, lower;
  *computed_size = 1;
  if (size > 0)
    remaining = (size - 1);
  while ((next = strchr(pointer, '%'))) {
    if ((dimension = (next - pointer)) > 0) {
      *computed_size += dimension;
      if ((lower = (dimension > remaining) ? remaining : dimension)) {
        remaining -= lower;
        if (target) {
          memcpy(target, pointer, lower);
          target += lower;
        }
      }
    }
    if ((pointer = (next + 1)))
      if ((last = p_string_format_skip(pointer, symbols))) {
        char argument[d_string_argument_size];
        size_t written;
        memset(argument, 0, d_string_argument_size);
        memcpy(argument, next, (last - next));
        if ((!symbols) || (!((tail = strchr(symbols, *(last - 1)))))) {
          switch (tolower(*(last - 1))) {
            case 'd':
            case 'i':
            case 'u':
            case 'x':
            case 'o':
            case 'n':
            case 'c':
              written = snprintf(target, remaining, argument, va_arg(parameters,
                long));
              break;
            case 'f':
            case 'e':
            case 'g':
              written = snprintf(target, remaining, argument, va_arg(parameters,
                double));
              break;
            case 's':
            case 'p':
              written = snprintf(target, remaining, argument, va_arg(parameters,
                void *));
              break;
            default:
              written = 0;
          }
        } else
          written = functions[(tail - symbols)](target, remaining, argument,
            parameters);
        *computed_size += written;
        written = ((written > remaining) ? remaining : written);
        remaining -= written;
        if (target)
          target += written;
        pointer = last;
      }
  }
  if ((dimension = strlen(pointer)) > 0) {
    *computed_size += dimension;
    if ((lower = (dimension > remaining) ? remaining : dimension)) {
      remaining -= lower;
      if (target) {
        memcpy(target, pointer, lower);
        target += lower;
      }
    }
  }
  if (target)
    *target = '\0';
  return buffer;
}
char *f_string_format_malloc(const char *symbols, t_string_formatter functions[], char *format, ...) {
  va_list parameters;
  char *result = NULL;
  va_start(parameters, format);
  {
    result = f_string_format_malloc_args(symbols, functions, format, parameters);
  }
  va_end(parameters);
  return result;
}
char *f_string_format_malloc_args(const char *symbols, t_string_formatter functions[], char *format, va_list parameters) {
  char *result = NULL;
  size_t required_size;
  va_list parameters_backup;
  va_copy(parameters_backup, parameters);
  {
    f_string_format_args(NULL, &required_size, 0, symbols, functions, format, parameters);
    if ((result = (char *)d_malloc(required_size + 1))) {
      size_t written_size;
      f_string_format_args(result, &written_size, (required_size + 1), symbols, functions, format, parameters_backup);
    }
  }
  va_end(parameters_backup);
  return result;
}