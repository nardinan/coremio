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
#include "../include/coremio/memory.h"
s_list m_memory_chunks;
void *f_memory_malloc(const char *file, const size_t line, const size_t size) {
  s_memory_node *result;
  if ((result = (s_memory_node *)malloc(sizeof(s_memory_node) + size))) {
    memset(&(result->head), 0, sizeof(s_list_node));
    result->file = file;
    result->line = line;
    result->size = size;
    f_list_append(&m_memory_chunks, (s_list_node *)result, e_list_insert_head);
    result  = ((void *)result) + sizeof(s_memory_node);
  }
  return result;
}
void *f_memory_realloc(const char *file, const size_t line, void *pointer, const size_t size) {
  void *new_pointer;
  if ((new_pointer = f_memory_malloc(file, line, size))) {
    if (pointer) {
      const s_memory_node *previous_memory_node = pointer - sizeof(s_memory_node);
      memcpy(new_pointer, pointer, previous_memory_node->size);
      d_free(pointer);
    }
  } else
    new_pointer = pointer; /* in case of error, we return the old pointer */
  return new_pointer;
}
void f_memory_free(void *pointer) {
  if (pointer) {
    s_memory_node *memory_node = pointer - sizeof(s_memory_node);
    f_list_remove(&m_memory_chunks, (s_list_node *)memory_node);
    free(memory_node);
  }
}
void f_memory_print_plain(void) {
  s_memory_node *memory_node;
  d_list_foreach(&m_memory_chunks, memory_node, s_memory_node) {
    printf("[%s @ %zu (size %zu)] still here\n",
      memory_node->file,
      memory_node->line,
      memory_node->size);
  }
}