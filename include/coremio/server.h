/**
 * MIT License
 * Copyright (c) [2026] The Barfing Fox - TBF [nardinan (andrea@nardinan.it)]
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
#ifndef SERVER_H
#define SERVER_H
#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include "list.h"
#include "result.h"
#include "runner.h"
#define d_server_frequency_sleep_milliseconds 250
d_result_declare(SHIT_SOCKET_CREATE);
d_result_declare(SHIT_SOCKET_SET_OPTION);
d_result_declare(SHIT_SOCKET_BIND);
d_result_declare(SHIT_SOCKET_LISTEN_MODE);
d_result_declare(SHIT_SOCKET_CONNECT);
d_result_declare(SHIT_SOCKET_DISCONNECTED);
extern coremio_result f_socket_create_server(unsigned short int port, unsigned short int queue, int *descriptor, struct sockaddr_in *configuration);
extern coremio_result f_socket_create_client(unsigned short int port, const char *address, int *descriptor, struct sockaddr_in *configuration);
extern coremio_result f_socket_read(int descriptor, unsigned char *in_buffer, size_t buffer_size, size_t *read_size, time_t timeout_milliseconds);
extern coremio_result f_socket_write(int descriptor, unsigned char *out_buffer, size_t buffer_size, size_t *write_size, time_t timeout_milliseconds);
extern void f_socket_close(int *descriptor);
typedef struct s_server_connection_node {
  s_list_node head;
  struct sockaddr_in connection_socket_address;
  int connection_descriptor;
} s_server_connection_node;
typedef struct s_server {
  s_runner head;
  unsigned short int port;
  struct sockaddr_in configuration;
  size_t connection_node_size;
  int descriptor;
  pthread_mutex_t connections_lock;
  s_list connections;
} s_server;
extern coremio_result f_server_initialize(s_server *server, unsigned short int port, size_t connection_node_size);
extern coremio_result f_server_run(s_server *server, unsigned short int queue);
extern s_server_connection_node *f_server_get_connection_node(s_server *server);
extern void f_server_connection_free(s_server_connection_node *connection);
extern void f_server_free(s_server *server);
#endif // SERVER_H
