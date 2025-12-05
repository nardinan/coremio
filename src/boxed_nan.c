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
#include "../include/coremio/boxed_nan.h"
double f_boxed_nan_boolean(const bool value) {
  u_boxed_nan_container result;
  result.integer_value = ((int64_t)d_boxed_nan_bool_signature << 48) | ((value) ? 0x1 : 0x0);
  return result.double_value;
}
double f_boxed_nan_int(const int32_t value) {
  u_boxed_nan_container result;
  result.integer_value = ((int64_t)d_boxed_nan_int_signature << 48) | (value & 0xFFFFFFFF);
  return result.double_value;
}
double f_boxed_nan_embedded_string(const char *value, size_t length) {
  u_boxed_nan_container result;
  const size_t length_string = ((length > 0) ? length : strlen(value));
  result.integer_value = ((int64_t)d_boxed_nan_embedded_string_signature << 48);
  for (size_t index = 0; ((index < length_string) && (index < (d_boxed_nan_available_bytes - 1))); ++index)
    result.integer_value |= (((int64_t)(((index < length_string) ? value[index] : 0) & 0xFF)) << (8 * index)) & d_boxed_nan_mask_payload;
  return result.double_value;
}
static u_boxed_nan_container p_boxed_nan_pointer_generic(const int64_t signature, void *value) {
  u_boxed_nan_container result;
  /* nardinan NOTE:
   * warning, this doesn't work if your memory space goes above ~256TB */
  result.integer_value = (signature << 48) | ((int64_t)value & d_boxed_nan_mask_payload);
  return result;
}
double f_boxed_nan_pointer_string(const char *value) {
  const u_boxed_nan_container result = p_boxed_nan_pointer_generic(d_boxed_nan_pointer_string_signature, (void *)value);
  return result.double_value;
}
double f_boxed_nan_string(const char *value) {
  const size_t length = strlen(value);
  return ((length < d_boxed_nan_available_bytes - 1) ? f_boxed_nan_embedded_string(value, length) : f_boxed_nan_pointer_string(value));
}
double f_boxed_nan_pointer_custom(void *value) {
  const u_boxed_nan_container result = p_boxed_nan_pointer_generic(d_boxed_nan_pointer_custom_signature, value);
  return result.double_value;
}
void f_boxed_nan_get_embedded_string(const double value, char *storage) {
  const u_boxed_nan_container encoded_value = (u_boxed_nan_container){ .double_value = value };
  strncpy(storage, (char *)(&(encoded_value.integer_value)), d_boxed_nan_available_bytes);
}
static size_t p_boxed_nan_string_formatter_boolean(char *target, const size_t size, const double entry) {
  return snprintf(target, size + 1, "boolean (value: %s)", (d_boxed_nan_get_boolean(entry)?"true":"false"));
}
static size_t p_boxed_nan_string_formatter_int(char *target, const size_t size, const double entry) {
  return snprintf(target, size + 1, "int (value: %d)", d_boxed_nan_get_int(entry));
}
static size_t p_boxed_nan_string_formatter_embedded_string(char *target, const size_t size, const double entry) {
  char embedded_string[d_boxed_nan_available_bytes] = { 0 };
  f_boxed_nan_get_embedded_string(entry, embedded_string);
  return snprintf(target, size + 1, "string, embedded (value: \"%s\")", embedded_string);
}
static size_t p_boxed_nan_string_formatter_pointer_char(char *target, const size_t size, const double entry) {
  return snprintf(target, size + 1, "string (value: \"%s\")", (char *)d_boxed_nan_get_pointer(entry));
}
static size_t p_boxed_nan_string_formatter_pointer_custom(char *target, const size_t size, const double entry) {
  return snprintf(target, size + 1, "pointer (value: \"%p\")", d_boxed_nan_get_pointer(entry));
}
size_t f_boxed_nan_string_formatter(char *target, const size_t size, char *symbol, va_list parameters) {
  double value;
  size_t written = 0;
  if ((value = va_arg(parameters, double))) {
    switch (d_boxed_nan_get_signature(value)) {
      case d_boxed_nan_bool_signature: {
        written = p_boxed_nan_string_formatter_boolean(target, size, value);
        break;
      }
      case d_boxed_nan_int_signature: {
        written = p_boxed_nan_string_formatter_int(target, size, value);
        break;
      }
      case d_boxed_nan_embedded_string_signature: {
        written = p_boxed_nan_string_formatter_embedded_string(target, size, value);
        break;
      }
      case d_boxed_nan_pointer_string_signature: {
        written = p_boxed_nan_string_formatter_pointer_char(target, size, value);
        break;
      }
      case d_boxed_nan_pointer_custom_signature: {
        written = p_boxed_nan_string_formatter_pointer_custom(target, size, value);
        break;
      }
      case d_boxed_nan_nan_signature:
      default: {
        written = snprintf(target, size + 1, "just a double (value: %f)", value);
      }
    }
  }
  return written;
}