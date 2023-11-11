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
#ifndef COREMIO_DICTIONARY_H
#define COREMIO_DICTIONARY_H
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include "red_black_tree.h"
typedef struct s_dictionary_node {
  s_red_black_tree_node head;
  struct s_dictionary *owner;
  char *key;
} s_dictionary_node;
typedef void (*l_dictionary_node_initialize)(s_dictionary_node *);
typedef void (*l_dictionary_node_delete)(s_dictionary_node *);
typedef void (*l_dictionary_node_visit)(s_dictionary_node *, void *);
typedef struct s_dictionary {
  s_red_black_tree head;
  l_dictionary_node_initialize f_dictionary_node_initialize;
  l_dictionary_node_delete f_dictionary_node_delete;
  size_t node_size;
} s_dictionary;
void f_dictionary_initialize_custom(s_dictionary *dictionary, size_t node_size, l_dictionary_node_initialize f_dictionary_node_initialize,
  l_dictionary_node_delete f_dictionary_node_delete);
void f_dictionary_initialize(s_dictionary *dictionary, size_t node_size);
s_dictionary_node *f_dictionary_get_informed(s_dictionary *dictionary, const char *key, bool *is_created);
s_dictionary_node *f_dictionary_get(s_dictionary *dictionary, const char *key);
void f_dictionary_foreach(const s_dictionary *dictionary, l_dictionary_node_visit f_dictionary_node_visit, void *payload);
void f_dictionary_free(s_dictionary *dictionary);
#endif //COREMIO_DICTIONARY_H
