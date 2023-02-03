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
#ifndef COREMIO_RED_BLACK_TREE_H
#define COREMIO_RED_BLACK_TREE_H
#include <stdio.h>
#include "memory.h"
typedef enum e_red_black_tree_colors {
  e_red_black_tree_color_red,
  e_red_black_tree_color_black
} e_red_black_tree_colors;
struct s_red_black_tree;
typedef struct s_red_black_tree_node {
  long int value;
  struct s_red_black_tree_node *parent, *left, *right;
  e_red_black_tree_colors color;
  struct s_red_black_tree *owner;
} s_red_black_tree_node;
typedef long int (*l_red_black_tree_evaluation)(s_red_black_tree_node *);
typedef void (*l_red_black_tree_node_delete)(s_red_black_tree_node *);
typedef struct s_red_black_tree {
  s_red_black_tree_node *root;
  l_red_black_tree_evaluation f_red_black_tree_evaluation;
  l_red_black_tree_node_delete f_red_black_tree_node_delete;
} s_red_black_tree;
extern void f_red_black_tree_insert(s_red_black_tree *red_black_tree, s_red_black_tree_node *node);
extern void f_red_black_tree_free(s_red_black_tree *red_black_tree);
#endif //COREMIO_RED_BLACK_TREE_H
