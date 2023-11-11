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
#include "../include/coremio/dictionary.h"
static long int p_dictionary_evaluate_string(const char *key) {
  long int hash = 5381;
  while (*key) {
    hash = ((hash << 5) + hash) + *key;
    ++key;
  }
  return hash;
}
static long int p_dictionary_evaluate(s_red_black_tree_node *node) {
  return p_dictionary_evaluate_string(((s_dictionary_node *)node)->key);
}
static void p_dictionary_node_delete(s_red_black_tree_node *node) {
  s_dictionary_node *dictionary_node = (s_dictionary_node *)node;
  if (dictionary_node->owner)
    if (dictionary_node->owner->f_dictionary_node_delete)
      dictionary_node->owner->f_dictionary_node_delete(dictionary_node);
  if (dictionary_node->key)
    d_free(dictionary_node->key);
  d_free(dictionary_node);
}
void f_dictionary_initialize_custom(s_dictionary *dictionary, const size_t node_size, const l_dictionary_node_initialize f_dictionary_node_initialize,
  const l_dictionary_node_delete f_dictionary_node_delete) {
  memset(dictionary, 0, sizeof(s_dictionary));
  dictionary->head.f_red_black_tree_evaluation = p_dictionary_evaluate;
  dictionary->head.f_red_black_tree_node_delete = p_dictionary_node_delete;;
  dictionary->f_dictionary_node_initialize = f_dictionary_node_initialize;
  dictionary->f_dictionary_node_delete = f_dictionary_node_delete;
  dictionary->node_size = node_size;
}
void f_dictionary_initialize(s_dictionary *dictionary, const size_t node_size) {
  f_dictionary_initialize_custom(dictionary, node_size, NULL, NULL);
}
static s_dictionary_node *p_dictionary_get_recursive(s_dictionary *dictionary, s_dictionary_node *node, const char *key, const long int evaluation) {
  s_dictionary_node *result = NULL;
  if (node) {
    if ((node->head.value == evaluation) && (strcmp(node->key, key) == 0))
      result = node;
    else if (evaluation < node->head.value)
      result = p_dictionary_get_recursive(dictionary, (s_dictionary_node *)node->head.left, key, evaluation);
    else
      result = p_dictionary_get_recursive(dictionary, (s_dictionary_node *)node->head.right, key, evaluation);
  }
  return result;
}
s_dictionary_node *f_dictionary_get_informed(s_dictionary *dictionary, const char *key, bool *is_created) {
  s_dictionary_node *result = p_dictionary_get_recursive(dictionary, (s_dictionary_node *)dictionary->head.root, key, p_dictionary_evaluate_string(key));
  *is_created = false;
  if (!result)
    if ((result = (s_dictionary_node *)d_malloc(dictionary->node_size))) {
      const size_t length_key = strlen(key);
      memset(result, 0, dictionary->node_size);
      if ((result->key = (char *)d_malloc(length_key + 1))) {
        strncpy(result->key, key, length_key);
        result->key[length_key] = 0;
        result->owner = dictionary;
        if (dictionary->f_dictionary_node_initialize)
          dictionary->f_dictionary_node_initialize(result);
        f_red_black_tree_insert((s_red_black_tree *)dictionary, (s_red_black_tree_node *)result);
        *is_created = true;
      } else {
        d_free(result);
        result = NULL;
      }
    }
  return result;
}
s_dictionary_node *f_dictionary_get(s_dictionary *dictionary, const char *key) {
  bool is_created;
  return f_dictionary_get_informed(dictionary, key, &is_created);
}
static void p_dictionary_foreach_pre_order(s_dictionary_node *current_node, const l_dictionary_node_visit f_dictionary_node_visit, void *payload) {
  if (current_node) {
    if (f_dictionary_node_visit)
      f_dictionary_node_visit(current_node, payload);
    p_dictionary_foreach_pre_order((s_dictionary_node *)current_node->head.left, f_dictionary_node_visit, payload);
    p_dictionary_foreach_pre_order((s_dictionary_node *)current_node->head.right, f_dictionary_node_visit, payload);
  }
}
void f_dictionary_foreach(const s_dictionary *dictionary, const l_dictionary_node_visit f_dictionary_node_visit, void *payload) {
  p_dictionary_foreach_pre_order((s_dictionary_node *)dictionary->head.root, f_dictionary_node_visit, payload);
}
void f_dictionary_free(s_dictionary *dictionary) {
  f_red_black_tree_free(&(dictionary->head));
}