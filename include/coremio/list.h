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
#ifndef COREMIO_LIST_H
#define COREMIO_LIST_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "assert.h"
#define d_list_foreach(l,n,t) for((n)=(t*)((l)->head);(n);(n)=(t*)((struct s_list_node *)(n))->next)
#define d_list_foreach_reverse(l,n,t) for((n)=(t*)((l)->tail);(n);(n)=(t*)((struct s_list_node *)(n))->previous)
#define d_list_safe_next(n) ((n)?((s_list_node *)(n))->next:NULL)
typedef enum e_list_insert_kind {
  e_list_insert_head,
  e_list_insert_tail
} e_list_insert_kind;
struct s_list_node;
typedef struct s_list {
  struct s_list_node *head, *tail;
  size_t entries;
} s_list;
typedef struct s_list_node {
  s_list *owner;
  struct s_list_node *next, *previous;
} s_list_node;
typedef bool (* l_list_to_swap)(s_list_node *, s_list_node *);
extern void f_list_append(s_list *list, s_list_node *node, e_list_insert_kind kind);
extern void f_list_insert(s_list *list, s_list_node *node, s_list_node *previous);
extern void f_list_sort(s_list *list, l_list_to_swap comparison);
extern s_list_node *f_list_remove(s_list *list, s_list_node *node);
extern s_list_node *f_list_remove_from_owner(s_list_node *node);
#endif //COREMIO_LIST_H
