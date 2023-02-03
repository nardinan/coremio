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
#ifndef COREMIO_ASSERT_H
#define COREMIO_ASSERT_H
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#define d_dump_debug(format...)\
  do{                           \
    fprintf(stdout,"[%s@%d] -", __FILE__, __LINE__);\
    fprintf(stdout,format);\
    fflush(stdout);\
  }while(0)
#define d_assert(expression)\
  do{\
    if (!(expression)){\
      d_dump_debug(#expression);\
      abort();\
    }\
  }while(0)
extern void f_(const char *file, unsigned int line, const char *condition);
#endif //COREMIO_ASSERT_H
