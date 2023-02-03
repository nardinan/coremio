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
#include "../include/coremio/list.h"
void f_list_append(s_list *list, s_list_node *node, e_list_insert_kind kind) {
  d_assert((node->owner == NULL));
  switch (kind) {
    case e_list_insert_head:
      if (list->head)
        list->head->previous = node;
      else
        list->tail = node;
      node->next = list->head;
      list->head = node;
      break;
    case e_list_insert_tail:
      if (list->tail)
        list->tail->next = node;
      else
        list->head = node;
      node->previous = list->tail;
      list->tail = node;
  }
  node->owner = list;
  ++(list->entries);
}
void f_list_insert(s_list *list, s_list_node *node, s_list_node *previous) {
  d_assert((node->owner == NULL));
  if (previous) {
    if (previous->next) {
      node->next = previous->next;
      node->next->previous = node;
      node->previous = previous;
      previous->next = node;
      node->owner = list;
      ++(list->entries);
    } else
      f_list_append(list, node, e_list_insert_tail);
  } else
    f_list_append(list, node, e_list_insert_head);
}
void f_list_sort(s_list *list, l_list_to_swap comparison) {
  if (list->head) {
    s_list_node *original_head = list->head->next;
    list->head->previous = NULL;
    list->head->next = NULL;
    list->tail = list->head;
    list->entries = 1;
    while (original_head) {
      s_list_node *current_sorted_node = list->head, *next_original_head = original_head->next;
      original_head->next = NULL;
      original_head->previous = NULL;
      original_head->owner = NULL;
      while (current_sorted_node) {
        if (comparison(current_sorted_node, original_head)) {
          f_list_insert(list, original_head, current_sorted_node->previous);
          break;
        } else if (!current_sorted_node->next) {
          f_list_insert(list, original_head, current_sorted_node);
          break;
        } else
          current_sorted_node = current_sorted_node->next;
      }
      original_head = next_original_head;
    }
  }
}
s_list_node *f_list_remove(s_list *list, s_list_node *node) {
  d_assert((node->owner == list));
  if (node->owner == list) {
    if (node->next)
      node->next->previous = node->previous;
    else
      list->tail = node->previous;
    if (node->previous)
      node->previous->next = node->next;
    else
      list->head = node->next;
    node->next = NULL;
    node->previous = NULL;
    node->owner = NULL;
    --(list->entries);
  }
  return node;
}
s_list_node *f_list_remove_from_owner(s_list_node *node) {
  return f_list_remove(node->owner, node);
}