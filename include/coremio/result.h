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
#ifndef COREMIO_RESULT_H
#define COREMIO_RESULT_H
#include <stdio.h>
#include <stdarg.h>
#include <libgen.h>
#define d_result_declare(rc) extern struct s_result *rc
#define d_result_define(rc,cod,d) struct s_result _##rc = {#rc,d,__FILE__,cod}, *rc = &_##rc
struct s_result;
d_result_declare(NOICE);
d_result_declare(SHIT);
d_result_declare(SHIT_AGAIN);
d_result_declare(SHIT_INVALID_PARAMETERS);
d_result_declare(SHIT_NOT_INITIALIZED);
d_result_declare(SHIT_ALREADY_INITIALIZED);
d_result_declare(SHIT_NOT_FOUND);
d_result_declare(SHIT_NO_MEMORY);
d_result_declare(SHIT_MALFORMED_STRUCTURE);
d_result_declare(SHIT_NO_ANSWER);
typedef struct s_result {
  char *name, *description, *environment;
  unsigned int code;
} s_result;
typedef s_result * coremio_result;
extern size_t f_result_string_formatter(char *target, size_t size, char *symbol, va_list parameters);
#endif //COREMIO_RESULT_H
