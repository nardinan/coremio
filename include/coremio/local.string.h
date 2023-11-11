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
#ifndef COREMIO_LOCAL_STRING_H
#define COREMIO_LOCAL_STRING_H
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>
#include "memory.h"
#define d_string_argument_size 24
#define d_string_buffer_size 64
#define d_space_character(c) (((c)==' ')||((c)=='\t'))
#define d_final_character(c) (((c)=='\0')||((c)=='\n')||((c)=='\r'))
typedef size_t (*t_string_formatter)(char *, size_t, char *, va_list);
extern char *f_string_trim(char *string);
extern char *f_string_format(char *buffer, size_t *computed_size, size_t size, const char *symbols, t_string_formatter functions[], char *format, ...);
extern char *f_string_format_args(char *buffer, size_t *computed_size, size_t size, const char *symbols, t_string_formatter functions[], char *format,
  va_list parameters);
extern char *f_string_format_malloc(const char *symbols, t_string_formatter functions[], char *format, ...) __attribute__((malloc));
extern char *f_string_format_malloc_args(const char *symbols, t_string_formatter functions[], char *format, va_list parameters) __attribute__((malloc));
#endif //COREMIO_LOCAL_STRING_H
