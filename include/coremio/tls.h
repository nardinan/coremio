/**
 * MIT License
 * Copyright (c) [2026] The Barfing Fox [Andrea Nardinocchi (andrea@nardinan.it)]
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
#ifndef COREMIO_TLS_H
#define COREMIO_TLS_H
#include <openssl/err.h>
#include <openssl/ssl.h>
#include "result.h"
#include "server.h"
d_result_declare(SHIT_TLS_CONTEXT_CREATION);
d_result_declare(SHIT_TLS_HANDSHAKE);
d_result_declare(SHIT_TLS_CERTIFICATE);
typedef struct s_client_socket_SSL {
  SSL_CTX *context;
  SSL *session;
  int descriptor;
} s_client_socket_SSL;
extern coremio_result f_socket_create_client_ssl(unsigned short int port, const char *address, s_client_socket_SSL *descriptor, const char *ca_file,
    time_t timeout_milliseconds);
extern coremio_result f_socket_read_ssl(const s_client_socket_SSL *descriptor, unsigned char *in_buffer, size_t buffer_size, size_t *read_size,
    time_t timeout_milliseconds);
extern coremio_result f_socket_write_ssl(const s_client_socket_SSL *descriptor, unsigned char *out_buffer, size_t buffer_size, size_t *write_size,
    time_t timeout_milliseconds);
extern void f_socket_close_ssl(s_client_socket_SSL *descriptor);
#endif // COREMIO_TLS_H
