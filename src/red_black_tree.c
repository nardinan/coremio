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
#include "../include/coremio/red_black_tree.h"
static void p_red_black_tree_rotate_right(s_red_black_tree *red_black_tree, s_red_black_tree_node *pivot) {
  s_red_black_tree_node *left = pivot->left;
  if ((pivot->left = left->right))
    pivot->left->parent = pivot;
  if (!((left->parent = pivot->parent)))
    red_black_tree->root = left;
  else if (pivot == pivot->parent->left)
    pivot->parent->left = left;
  else
    pivot->parent->right = left;
  left->right = pivot;
  pivot->parent = left;
}
static void p_red_black_tree_rotate_left(s_red_black_tree *red_black_tree, s_red_black_tree_node *pivot) {
  s_red_black_tree_node *right = pivot->right;
  if ((pivot->right = right->left))
    pivot->right->parent = pivot;
  if (!((right->parent = pivot->parent)))
    red_black_tree->root = right;
  else if (pivot == pivot->parent->left)
    pivot->parent->left = right;
  else
    pivot->parent->right = right;
  right->left = pivot;
  pivot->parent = right;
}
static void p_red_black_tree_fix(s_red_black_tree *red_black_tree, const s_red_black_tree_node *node) {
  while ((node != red_black_tree->root) && (node->color != e_red_black_tree_color_black) &&
         (node->parent->color == e_red_black_tree_color_red)) {
    s_red_black_tree_node *parent = node->parent, *grand_parent = node->parent->parent;
    if (grand_parent->left == parent) {
      s_red_black_tree_node *uncle = grand_parent->right;
      if ((uncle) && (uncle->color == e_red_black_tree_color_red)) {
        grand_parent->color = e_red_black_tree_color_red;
        parent->color = e_red_black_tree_color_black;
        uncle->color = e_red_black_tree_color_black;
        node = grand_parent;
      } else {
        e_red_black_tree_colors color;
        if (node == parent->right) {
          p_red_black_tree_rotate_left(red_black_tree, parent);
          node = parent;
          parent = node->parent;
        }
        p_red_black_tree_rotate_right(red_black_tree, grand_parent);
        color = parent->color;
        parent->color = grand_parent->color;
        grand_parent->color = color;
        node = parent;
      }
    } else {
      s_red_black_tree_node *uncle = grand_parent->left;
      if ((uncle) && (uncle->color == e_red_black_tree_color_red)) {
        grand_parent->color = e_red_black_tree_color_red;
        parent->color = e_red_black_tree_color_black;
        uncle->color = e_red_black_tree_color_black;
        node = grand_parent;
      } else {
        e_red_black_tree_colors color;
        if (node == parent->left) {
          p_red_black_tree_rotate_right(red_black_tree, parent);
          node = parent;
          parent = node->parent;
        }
        p_red_black_tree_rotate_left(red_black_tree, grand_parent);
        color = parent->color;
        parent->color = grand_parent->color;
        grand_parent->color = color;
        node = parent;
      }
    }
  }
  red_black_tree->root->color = e_red_black_tree_color_black;
}
static s_red_black_tree_node *p_red_black_tree_insert_recursive(s_red_black_tree_node *visiting, s_red_black_tree_node *node) {
  s_red_black_tree_node *result = visiting;
  if (visiting) {
    node->parent = visiting;
    if (node->value > visiting->value)
      visiting->right = p_red_black_tree_insert_recursive(visiting->right, node);
    else
      visiting->left = p_red_black_tree_insert_recursive(visiting->left, node);
  } else
    result = node;
  return result;
}
static unsigned int p_red_black_tree_depth(const s_red_black_tree_node *selected) {
  unsigned int result = 0;
  if (selected) {
    const unsigned int left_depth = 1 + p_red_black_tree_depth(selected->left),
      right_depth = 1 + p_red_black_tree_depth(selected->right);
    result = (left_depth > right_depth)?left_depth:right_depth;
  }
  return result;
}
void f_red_black_tree_insert(s_red_black_tree *red_black_tree, s_red_black_tree_node *node) {
  if (red_black_tree->f_red_black_tree_evaluation)
    node->value = red_black_tree->f_red_black_tree_evaluation(node);
  node->color = e_red_black_tree_color_red; /* new insertion is always red, rule number 3 */
  node->owner = red_black_tree;
  red_black_tree->root = p_red_black_tree_insert_recursive(red_black_tree->root, node);
  p_red_black_tree_fix(red_black_tree, node);
}
static void p_red_black_tree_free_recursive(s_red_black_tree *red_black_tree, s_red_black_tree_node *node) {
  if (node) {
    p_red_black_tree_free_recursive(red_black_tree, node->left);
    p_red_black_tree_free_recursive(red_black_tree, node->right);
    if (red_black_tree->f_red_black_tree_node_delete)
      red_black_tree->f_red_black_tree_node_delete(node);
  }
}
void f_red_black_tree_free(s_red_black_tree *red_black_tree) {
  p_red_black_tree_free_recursive(red_black_tree, red_black_tree->root);
}