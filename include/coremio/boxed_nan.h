/**
 * MIT License
 * Copyright (c) [2023] The Barfing Fox [Andrea Nardinocchi (andrea@nardinan.it)]
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
#ifndef COREMIO_BOXED_NAN_H
#define COREMIO_BOXED_NAN_H
#include <stdbool.h>
#include <string.h>
#include "result.h"
/* we're using the following encoding:
 * <13-1> 100 -> real NAN
 * <13-1> 001 -> boolean
 * <13-1> 010 -> integer
 * <13-1> 011 -> embedded string (max 5 + EOS character)
 * <13-1> 101 -> pointer to char
 * <13-1> 110 -> pointer to custom structure
 */
#define d_boxed_nan_nan_signature 0x7FF8
#define d_boxed_nan_bool_signature 0x7FF9
#define d_boxed_nan_int_signature 0x7FFA
#define d_boxed_nan_embedded_string_signature 0x7FFB
#define d_boxed_nan_pointer_string_signature 0x7FFC
#define d_boxed_nan_pointer_custom_signature 0x7FFD
#define d_boxed_nan_mask_signature 0x7FFF
#define d_boxed_nan_mask_payload 0xFFFFFFFFFFFF
#define d_boxed_nan_available_bytes 6 /* tag and NAN are 2 bytes */
typedef union u_boxed_nan_container {
  int64_t integer_value;
  double double_value;
} u_boxed_nan_container;
extern double f_boxed_nan_boolean(bool value);
extern double f_boxed_nan_int(int32_t value);
extern double f_boxed_nan_embedded_string(const char *value, size_t length);
extern double f_boxed_nan_pointer_string(const char *value);
extern double f_boxed_nan_string(const char *value);
extern double f_boxed_nan_pointer_custom(void *value);
#define d_boxed_nan_is_boxed_nan(d) (((((u_boxed_nan_container){.double_value=(d)}).integer_value >> 48) &\
  d_boxed_nan_nan_signature) == d_boxed_nan_nan_signature)
#define d_boxed_nan_get_signature(d) ((((u_boxed_nan_container){.double_value=(d)}).integer_value >> 48) & d_boxed_nan_mask_signature)
#define d_boxed_nan_get_boolean(d) ((bool)((((u_boxed_nan_container){.double_value=(d)}).integer_value & 0x1)))
#define d_boxed_nan_get_int(d) ((int32_t)((((u_boxed_nan_container){.double_value=(d)}).integer_value & d_boxed_nan_mask_payload)))
#define d_boxed_nan_get_pointer(d) ((void *)(((u_boxed_nan_container){.double_value=(d)}).integer_value & d_boxed_nan_mask_payload))
extern void f_boxed_nan_get_embedded_string(double value, char *storage);
extern size_t f_boxed_nan_string_formatter(char *target, size_t size, char *symbol, va_list parameters);
#endif //COREMIO_BOXED_NAN_H
