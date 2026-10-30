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
#include "../include/coremio/tls.h"
#include <stdbool.h>
#include <sys/unistd.h>
#include "coremio/result.h"
d_result_define(SHIT_TLS_CONTEXT_CREATION, 1, "Failure, it seems impossible to create a new SSL context");
d_result_define(SHIT_TLS_HANDSHAKE, 2, "Failure, it seems impossible to complete the SSL handshake procedure");
d_result_define(SHIT_TLS_CERTIFICATE, 3, "Failure, it seems impossible to load a specific certificate or set the default certificates location");
coremio_result f_socket_create_client_ssl(unsigned short int port, const char *address, s_client_socket_SSL *descriptor, const char *ca_file,
    time_t timeout_milliseconds) {
  coremio_result result = NOICE;
  memset(descriptor, 0, sizeof(s_client_socket_SSL));
  descriptor->descriptor = -1;
  if ((result = f_socket_create_client(port, address, &(descriptor->descriptor), timeout_milliseconds)) == NOICE) {
    if ((descriptor->context = SSL_CTX_new(TLS_client_method()))) {
      SSL_CTX_set_verify(descriptor->context, SSL_VERIFY_PEER, NULL);
      SSL_CTX_set_min_proto_version(descriptor->context, TLS1_2_VERSION);
      if ((SSL_CTX_set_default_verify_paths(descriptor->context) == 1) &&
          ((!ca_file) || ((SSL_CTX_load_verify_locations(descriptor->context, ca_file, NULL)) == 1))) {
        if ((descriptor->session = SSL_new(descriptor->context))) {
          if ((SSL_set_tlsext_host_name(descriptor->session, address) == 1) && (SSL_set1_host(descriptor->session, address) == 1) &&
              (SSL_set_fd(descriptor->session, descriptor->descriptor) == 1)) {
            bool waiting = true;
            struct timeval timeout = {(timeout_milliseconds / 1000), (timeout_milliseconds % 1000) * 1000};
            while ((result == NOICE) && (waiting)) {
              int status_ssl_connect;
              if ((status_ssl_connect = SSL_connect(descriptor->session)) != 1) {
                int ssl_error;
                if (((ssl_error = SSL_get_error(descriptor->session, status_ssl_connect)) == SSL_ERROR_WANT_READ) || (ssl_error == SSL_ERROR_WANT_WRITE)) {
                  fd_set operation_set;
                  FD_ZERO(&operation_set);
                  FD_SET(descriptor->descriptor, &operation_set);
                  if (select((descriptor->descriptor + 1), ((ssl_error == SSL_ERROR_WANT_READ) ? &operation_set : NULL),
                          ((ssl_error == SSL_ERROR_WANT_WRITE) ? &operation_set : NULL), NULL, &timeout) == 0)
                    result = SHIT_TIMEOUT;
                } else
                  result = SHIT_TLS_HANDSHAKE;
              } else
                waiting = false;
            }
          } else
            result = SHIT_TLS_HANDSHAKE;
          if (result != NOICE) {
            SSL_shutdown(descriptor->session);
            SSL_free(descriptor->session);
            descriptor->session = NULL;
          }
        } else
          result = SHIT_NO_MEMORY;
      } else
        result = SHIT_TLS_CERTIFICATE;
      if (result != NOICE) {
        SSL_CTX_free(descriptor->context);
        descriptor->context = NULL;
      }
    } else
      result = SHIT_NO_MEMORY;
    if (result != NOICE)
      f_socket_close(&(descriptor->descriptor));
  }
  return result;
}
coremio_result f_socket_read_ssl(const s_client_socket_SSL *descriptor, unsigned char *in_buffer, size_t buffer_size, size_t *read_size,
    time_t timeout_milliseconds) {
  coremio_result result = NOICE;
  *read_size = 0;
  if (buffer_size > 0) {
    struct timeval timeout = {(timeout_milliseconds / 1000), (timeout_milliseconds % 1000) * 1000};
    result = SHIT_AGAIN;
    while (result == SHIT_AGAIN) {
      int status_ssl_read;
      if ((status_ssl_read = SSL_read_ex(descriptor->session, (in_buffer + *read_size), (buffer_size - *read_size), read_size)) != 1) {
        int ssl_error;
        if (((ssl_error = SSL_get_error(descriptor->session, status_ssl_read)) == SSL_ERROR_WANT_READ) || (ssl_error == SSL_ERROR_WANT_WRITE)) {
          fd_set operation_set;
          FD_ZERO(&operation_set);
          FD_SET(descriptor->descriptor, &operation_set);
          if (select((descriptor->descriptor + 1), ((ssl_error == SSL_ERROR_WANT_READ) ? &operation_set : NULL),
                  ((ssl_error == SSL_ERROR_WANT_WRITE) ? &operation_set : NULL), NULL, &timeout) == 0)
            result = SHIT_TIMEOUT;
        } else
          result = SHIT_SOCKET_DISCONNECTED;
      } else
        result = NOICE;
    }
  }
  return result;
}
coremio_result f_socket_write_ssl(const s_client_socket_SSL *descriptor, unsigned char *out_buffer, size_t buffer_size, size_t *write_size,
    time_t timeout_milliseconds) {
  coremio_result result = NOICE;
  struct timeval timeout = {(timeout_milliseconds / 1000), (timeout_milliseconds % 1000) * 1000};
  *write_size = 0;
  while ((result == NOICE) && (*write_size < buffer_size)) {
    size_t current_write_size = 0;
    int status_ssl_write;
    if ((status_ssl_write = SSL_write_ex(descriptor->session, (out_buffer + *write_size), (buffer_size - *write_size), &current_write_size)) != 1) {
      int ssl_error;
      if (((ssl_error = SSL_get_error(descriptor->session, status_ssl_write)) == SSL_ERROR_WANT_READ) || (ssl_error == SSL_ERROR_WANT_WRITE)) {
        fd_set operation_set;
        FD_ZERO(&operation_set);
        FD_SET(descriptor->descriptor, &operation_set);
        if (select((descriptor->descriptor + 1), ((ssl_error == SSL_ERROR_WANT_READ) ? &operation_set : NULL),
                ((ssl_error == SSL_ERROR_WANT_WRITE) ? &operation_set : NULL), NULL, &timeout) == 0)
          result = SHIT_TIMEOUT;
      } else
        result = SHIT_SOCKET_DISCONNECTED;
    } else
      *write_size += current_write_size;
  }
  return result;
}
void f_socket_close_ssl(s_client_socket_SSL *descriptor) {
  if (descriptor->session) {
    SSL_shutdown(descriptor->session);
    SSL_free(descriptor->session);
    descriptor->session = NULL;
  }
  if (descriptor->context) {
    SSL_CTX_free(descriptor->context);
    descriptor->context = NULL;
  }
  f_socket_close(&(descriptor->descriptor));
}
