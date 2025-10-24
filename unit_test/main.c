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
#include <sys/stat.h>
#include <fcntl.h>
#include "../include/coremio/json.h"
int main(int argc, char *argv[]) {
  int stream;
  if ((stream = open("example.json", O_RDONLY)) > -1) {
    coremio_result result;
    s_json json;
    if ((result = f_json_explode_stream(stream, &json)) == NOICE) {
      for (size_t index = 0; index < d_array_size(json.tokens); ++index) {
        f_tokens_print_detailed(STDOUT_FILENO, json.tokens[index]);
        printf("\n");
      }
      printf("\n\n%f\n\n", f_json_get_value(&json, NULL, "sds", "characters", 4, "layer"));
      f_json_print_plain(STDOUT_FILENO, json.root, &json);
      f_json_free(&json);
    }
    close(stream);
    printf("\n -- memory\n");
    f_memory_print_plain();
  }
  return 0;
}