/**
 * MIT License
 * Copyright (c) [2025] The Barfing Fox [Andrea Nardinocchi (andrea@nardinan.it)]
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
#ifndef COREMIO_NISP_STANDARD_LIBRARY_H
#define COREMIO_NISP_STANDARD_LIBRARY_H
struct s_nisp_environment;
struct s_nisp_node;
struct s_nisp;
extern struct s_nisp_node *f_nisp_standard_library_multiply(struct s_nisp *nisp, struct s_nisp_environment *environment);
extern struct s_nisp_node *f_nisp_standard_library_divide(struct s_nisp *nisp, struct s_nisp_environment *environment);
extern struct s_nisp_node *f_nisp_standard_library_sum(struct s_nisp *nisp, struct s_nisp_environment *environment);
extern struct s_nisp_node *f_nisp_standard_library_subtract(struct s_nisp *nisp, struct s_nisp_environment *environment);
extern struct s_nisp_node *f_nisp_standard_library_printnl(struct s_nisp *nisp, struct s_nisp_environment *environment);
extern struct s_nisp_node *f_nisp_standard_library_print(struct s_nisp *nisp, struct s_nisp_environment *environment);
#endif //COREMIO_NISP_STANDARD_LIBRARY_H
