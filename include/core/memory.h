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
#ifndef CORE_MEMORY_H
#define CORE_MEMORY_H
#include "list.h"
#define d_malloc(s) f_memory_malloc(__FILE__, __LINE__, s)
#define d_realloc(p,s) f_memory_realloc(__FILE__,__LINE__,p,s)
#define d_free(p) f_memory_free(p)
typedef struct s_memory_node {
  struct s_list_node head;
  const char *file;
  size_t line, size;
} s_memory_node;
extern s_list m_memory_chunks;
extern void *f_memory_malloc(const char *file, size_t line, size_t size);
extern void *f_memory_realloc(const char *file, size_t line, void *pointer, size_t size);
extern void f_memory_free(void *pointer);
extern void f_memory_print_plain(void);
#endif //CORE_MEMORY_H
