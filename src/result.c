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
#include "../include/coremio/result.h"
d_result_define(NOICE, 1, "Success");
d_result_define(SHIT, 2, "Failure");
d_result_define(SHIT_AGAIN, 3, "Failure, please, try again");
d_result_define(SHIT_INVALID_PARAMETERS, 4, "Failure, one or more parameters used are invalid");
d_result_define(SHIT_NOT_INITIALIZED, 5, "Failure, the element has not been initialized");
d_result_define(SHIT_ALREADY_INITIALIZED, 6, "Failure, the element seems to be already initialized");
d_result_define(SHIT_NOT_FOUND, 7, "Failure, required element cannot be found");
d_result_define(SHIT_NO_MEMORY, 8, "Failure, it seems impossible to allocate more memory");
d_result_define(SHIT_MALFORMED_STRUCTURE, 9, "Failure, it seems that the structure doesn't match what is expected");
d_result_define(SHIT_NO_ANSWER, 10, "Failure waiting to receive an answer");
size_t f_result_string_formatter(char *target, const size_t size, char *symbol, va_list parameters) {
  coremio_result value;
  size_t written = 0;
  if ((value = (coremio_result)va_arg(parameters, void *))) {
    written = snprintf(target, size + 1, "%s (code %d::%s, %s)", value->name, value->code,
      basename(value->environment), value->description);
  }
  return written;
}