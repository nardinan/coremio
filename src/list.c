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
void f_list_append(s_list *list, s_list_node *node, const e_list_insert_kind kind) {
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
static s_list_node *p_list_node_hard_split(s_list_node *head) {
  s_list_node *twice_step = head, *single_step = head, *result = NULL;
  while ((twice_step) && (twice_step->next))
    if ((twice_step = twice_step->next->next))
      single_step = single_step->next;
  if (single_step)
    if ((result = single_step->next)) {
      single_step->next->previous = NULL;
      single_step->next = NULL;
    }
  return result;
}
static s_list_node *p_list_node_hard_merge(s_list_node *left_head, s_list_node *right_head, const l_list_to_swap comparison) {
  s_list_node *result = NULL;
  if ((left_head) && (right_head)) {
    if (comparison(left_head, right_head)) {
      if ((left_head->next = p_list_node_hard_merge(left_head->next, right_head, comparison)))
        left_head->next->previous = left_head;
      result = left_head;
    } else {
      if ((right_head->next = p_list_node_hard_merge(left_head, right_head->next, comparison)))
        right_head->next->previous = right_head;
      result = right_head;
    }
  } else if (left_head)
    result = left_head;
  else
    result = right_head;
  return result;
}
static s_list_node *p_list_node_merge_sort(s_list_node *head, const l_list_to_swap comparison) {
  s_list_node *result = head;
  if ((head) && (head->next)) {
    s_list_node *left_head = head, *right_head = p_list_node_hard_split(left_head);
    result = p_list_node_hard_merge(p_list_node_merge_sort(left_head, comparison), p_list_node_merge_sort(right_head, comparison), comparison);
  }
  return result;
}
void f_list_sort(s_list *list, const l_list_to_swap comparison) {
  list->head = p_list_node_merge_sort(list->head, comparison);
  while ((list->tail) && (list->tail->next))
    list->tail = list->tail->next;
}
s_list_node *f_list_remove(s_list *list, s_list_node *node) {
  d_assert((node->owner == list));
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
  return node;
}
s_list_node *f_list_remove_from_owner(s_list_node *node) {
  return f_list_remove(node->owner, node);
}
