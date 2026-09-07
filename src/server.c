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
#include "../include/coremio/server.h"
#include "../include/coremio/memory.h"
d_result_define(SHIT_SOCKET_CREATE, 1, "Failure: impossible to create a unix socket");
d_result_define(SHIT_SOCKET_SET_OPTION, 2, "Failure: impossible to set/get options to/from a socket");
d_result_define(SHIT_SOCKET_BIND, 3, "Failure: impossible to bind the socket to a specific confguration");
d_result_define(SHIT_SOCKET_LISTEN_MODE, 4, "Failure: impossible to set an existing unix socket in listen mode");
d_result_define(SHIT_SOCKET_CONNECT, 5, "Failure: impossible to connect the socket to the destination");
d_result_define(SHIT_SOCKET_DISCONNECTED, 6, "Failure: the remote host got disconnected");
coremio_result f_socket_create_server(unsigned short int port, unsigned short int queue, int *descriptor, struct sockaddr_in *configuration) {
  coremio_result result = NOICE;
  bzero((void *) configuration, sizeof(struct sockaddr_in));
  if ((*descriptor = socket(AF_INET, SOCK_STREAM, 0)) > 0) {
    int option_reuse_address = 1;
    if (setsockopt(*descriptor, SOL_SOCKET, SO_REUSEADDR, &option_reuse_address, sizeof(int)) == 0) {
      int flags;
      if ((flags = fcntl(*descriptor, F_GETFL, 0)) < 0)
        flags = 0;
      if (fcntl(*descriptor, F_SETFL, (flags | O_NONBLOCK)) == 0) {
        configuration->sin_family = AF_INET;
        configuration->sin_port = htons(port);
        configuration->sin_addr.s_addr = INADDR_ANY;
        if (bind(*descriptor, (struct sockaddr *) configuration, sizeof(struct sockaddr_in)) == 0) {
          if (listen(*descriptor, queue) == -1)
            result = SHIT_SOCKET_LISTEN_MODE;
        } else
          result = SHIT_SOCKET_BIND;
      } else
        result = SHIT_SOCKET_SET_OPTION;
    } else
      result = SHIT_SOCKET_SET_OPTION;
    if (result != NOICE) {
      close(*descriptor);
      *descriptor = -1;
    }
  } else
    result = SHIT_SOCKET_CREATE;
  return result;
}
coremio_result f_socket_create_client(unsigned short int port, const char *address, int *descriptor, struct sockaddr_in *configuration) {
  coremio_result result = NOICE;
  bzero((void *) configuration, sizeof(struct sockaddr_in));
  if ((*descriptor = socket(AF_INET, SOCK_STREAM, 0)) > 0) {
    configuration->sin_family = AF_INET;
    configuration->sin_port = htons(port);
    if (inet_pton(AF_INET, address, &(configuration->sin_addr.s_addr)) > 0) {
      if (connect(*descriptor, (struct sockaddr *) configuration, sizeof(struct sockaddr_in)) == 0) {
        int flags;
        if ((flags = fcntl(*descriptor, F_GETFL, 0)) < 0)
          flags = 0;
        if (fcntl(*descriptor, F_SETFL, (flags | O_NONBLOCK)) != 0)
          result = SHIT_SOCKET_SET_OPTION;
      } else
        result = SHIT_SOCKET_CONNECT;
    }
    if (result != NOICE) {
      close(*descriptor);
      *descriptor = -1;
    }
  } else
    result = SHIT_SOCKET_CREATE;
  return result;
}
coremio_result f_socket_read(int descriptor, unsigned char *in_buffer, size_t buffer_size, size_t *read_size, time_t timeout_milliseconds) {
  coremio_result result = NOICE;
  size_t total_received_size = 0;
  ssize_t current_received_size = 0;
  struct timeval timeout = {(timeout_milliseconds / 1000), ((timeout_milliseconds % 1000) * 1000)};
  bool still_reading;
  do {
    fd_set socket_set;
    int select_status;
    FD_ZERO(&socket_set);
    FD_SET(descriptor, &socket_set);
    still_reading = false;
    if ((select_status = select(descriptor + 1, &socket_set, NULL, NULL, ((timeout_milliseconds > 0) ? &timeout : NULL))) > 0) {
      if ((current_received_size = read(descriptor, (in_buffer + total_received_size), (buffer_size - total_received_size))) > 0) {
        if ((size_t) current_received_size < (buffer_size - total_received_size))
          total_received_size += (size_t) current_received_size;
        else
          total_received_size = buffer_size;
      } else if (current_received_size < 0) {
        if ((errno != EAGAIN) && (errno != EWOULDBLOCK))
          result = SHIT_SOCKET_DISCONNECTED;
        else
          still_reading = true;
      } else
        result = SHIT_SOCKET_DISCONNECTED;
    } else if (select_status == -1) {
      if ((errno == EINVAL) || (errno == ENOMEM))
        result = SHIT_TIMEOUT;
      else
        result = SHIT_SOCKET_DISCONNECTED;
    } else
      result = SHIT_TIMEOUT;
  } while ((result == NOICE) && (still_reading));
  *read_size = total_received_size;
  return result;
}
coremio_result f_socket_write(int descriptor, unsigned char *out_buffer, size_t buffer_size, size_t *write_size, time_t timeout_milliseconds) {
  coremio_result result = NOICE;
  size_t total_sent_size = 0;
  ssize_t current_sent_size = 0;
  struct timeval timeout = {(timeout_milliseconds / 1000), ((timeout_milliseconds % 1000) * 1000)};
  bool still_reading;
  do {
    fd_set socket_set;
    int select_status;
    FD_ZERO(&socket_set);
    FD_SET(descriptor, &socket_set);
    if ((select_status = select(descriptor + 1, &socket_set, NULL, NULL, ((timeout_milliseconds > 0) ? &timeout : NULL))) > 0) {
      if ((current_sent_size = send(descriptor, (out_buffer + total_sent_size), (buffer_size - total_sent_size), MSG_NOSIGNAL)) > 0) {
        if ((size_t) current_sent_size < (buffer_size - total_sent_size))
          total_sent_size += (size_t) current_sent_size;
        else
          total_sent_size = buffer_size;
      } else if ((current_sent_size < 0) && (errno != EAGAIN) && (errno != EWOULDBLOCK))
        result = SHIT_SOCKET_DISCONNECTED;
    } else if (select_status == -1) {
      if ((errno == EINVAL) || (errno == ENOMEM))
        result = SHIT_TIMEOUT;
      else
        result = SHIT_SOCKET_DISCONNECTED;
    } else
      result = SHIT_TIMEOUT;
  } while ((result == NOICE) && (still_reading));
  *write_size = total_sent_size;
  return result;
}
static void p_server_run_callback(s_server *server, void *user_data) {
  bool kill_required = false;
  struct timeval reference_timeout = {(d_server_frequency_sleep_milliseconds / 1000), (d_server_frequency_sleep_milliseconds % 1000) * 1000}, current_timeout;
  struct sockaddr_in incoming_socket_address;
  socklen_t socket_address_size = sizeof(struct sockaddr_in);
  bool valid_socket = true;
  while ((!kill_required) && (valid_socket)) {
    fd_set socket_set;
    int incoming_descriptor, select_status;
    current_timeout = reference_timeout;
    FD_ZERO(&socket_set);
    FD_SET(server->descriptor, &socket_set);
    if ((select_status = select(server->descriptor + 1, &socket_set, NULL, NULL, &current_timeout)) > 0) {
      bzero((void *) &(incoming_socket_address), socket_address_size);
      if ((incoming_descriptor = accept(server->descriptor, (struct sockaddr *) &(incoming_socket_address), &socket_address_size)) > 0) {
        int flags;
        if ((flags = fcntl(incoming_descriptor, F_GETFL, 0)) < 0)
          flags = 0;
        if (fcntl(incoming_descriptor, F_SETFL, (flags | O_NONBLOCK)) == 0) {
          s_server_connection_node *connection_node = NULL;
          if ((connection_node = (s_server_connection_node *) d_malloc(server->connection_node_size))) {
            memset(connection_node, 0, server->connection_node_size);
            connection_node->connection_descriptor = incoming_descriptor;
            memcpy(&(connection_node->connection_socket_address), &(incoming_socket_address), socket_address_size);
            pthread_mutex_lock(&(server->connections_lock));
            {
              f_list_append(&(server->connections), (s_list_node *) connection_node, e_list_insert_tail);
            }
            pthread_mutex_unlock(&(server->connections_lock));
          }
          if (!connection_node) {
            shutdown(incoming_descriptor, SHUT_RDWR);
            close(incoming_descriptor);
          }
        }
      }
    } else if (select_status == -1)
      valid_socket = false;
    d_runner_is_interrupt_required(server, kill_required);
  }
}
coremio_result f_server_initialize(s_server *server, unsigned short int port, size_t connection_node_size) {
  coremio_result result;
  memset(server, 0, sizeof(s_server));
  if ((result = f_runner_initialize((s_runner *) server, (l_runner_user_callback) p_server_run_callback)) == NOICE) {
    if (pthread_mutex_init(&(server->connections_lock), NULL) == 0) {
      server->port = port;
      if ((server->connection_node_size = connection_node_size) < sizeof(s_server_connection_node))
        server->connection_node_size = sizeof(s_server_connection_node);
    } else {
      f_runner_free((s_runner *) server, false);
      result = SHIT_NO_MEMORY;
    }
  }
  return result;
}
coremio_result f_server_run(s_server *server, unsigned short int queue) {
  coremio_result result = NOICE;
  if (!f_runner_is_running((s_runner *) server)) {
    if ((result = f_socket_create_server(server->port, queue, &(server->descriptor), &(server->configuration))) == NOICE)
      f_runner_run((s_runner *) server, NULL);
  } else
    result = SHIT_ALREADY_INITIALIZED;
  return result;
}
s_server_connection_node *f_server_get_connection_node(s_server *server) {
  s_server_connection_node *result = NULL;
  pthread_mutex_lock(&(server->connections_lock));
  {
    if ((result = (s_server_connection_node *) server->connections.head))
      f_list_remove_from_owner((s_list_node *) result);
  }
  pthread_mutex_unlock(&(server->connections_lock));
  return result;
}
void f_server_connection_free(s_server_connection_node *connection) {
  shutdown(connection->connection_descriptor, SHUT_RDWR);
  close(connection->connection_descriptor);
  connection->connection_descriptor = -1;
}
void f_server_free(s_server *server) {
  f_runner_stop((s_runner *) server);
  f_runner_join((s_runner *) server);
  if (server->descriptor != -1) {
    pthread_mutex_lock(&(server->connections_lock));
    {
      s_server_connection_node *root;
      while ((root = (s_server_connection_node *) (server->connections.head))) {
        f_server_connection_free(root);
        f_list_remove_from_owner((s_list_node *) root);
        d_free(root);
      }
    }
    pthread_mutex_unlock(&(server->connections_lock));
    pthread_mutex_destroy(&(server->connections_lock));
    close(server->descriptor);
    server->descriptor = -1;
  }
}
