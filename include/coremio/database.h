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
#ifndef COREMIO_DATABASE_H
#define COREMIO_DATABASE_H
#include <pthread.h>
#include <sqlite3.h>
#include "list.h"
#include "tokens.h"
d_result_declare(SHIT_DATABASE_INVALID_PARAMETER);
d_result_declare(SHIT_DATABASE_INVALID_OPEN);
d_result_declare(SHIT_DATABASE_INVALID_QUERY);
typedef enum e_database_initialize_flags {
  e_database_initialize_flag_open_readwrite = SQLITE_OPEN_READWRITE,
  e_database_initialize_flag_open_readwrite_create = (SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE),
  e_database_initialize_flag_open_memory = SQLITE_OPEN_MEMORY,
  e_database_initialize_flag_open_URI = SQLITE_OPEN_URI,
  e_database_initialize_flag_open_full_mutex = SQLITE_OPEN_FULLMUTEX,
  e_database_initialize_flag_open_delete_when_close = SQLITE_OPEN_DELETEONCLOSE,
} e_database_initialize_flags;
typedef struct s_database_query_value {
  s_list_node head;
  t_token token;
  size_t token_size;
} s_database_query_value;
typedef struct s_database_query {
  s_list_node head;
  char *query;
  s_list values;
  size_t values_count;
} s_database_query;
struct s_database;
typedef void (*l_database_row)(struct s_database *database, s_database_query *query, size_t query_index, int columns_count, const char **keys,
    const char **values, void *user_data);
typedef struct s_database {
  sqlite3 *database;
  pthread_mutex_t database_lock;
  struct {
    const char **keys, **values;
    size_t columns_count;
  } cache_row;
} s_database;
extern coremio_result f_database_initialize(s_database *database, const char *path, const int flags, const time_t busy_timeout_milliseconds);
extern coremio_result f_database_run_raw_command(s_database *database, const char *command);
extern coremio_result f_database_query_prepare_args(s_database_query *database_query, const char *query, const char *types, va_list parameters);
extern coremio_result f_database_query_prepare(s_database_query *database_query, const char *query, const char *types, ...);
extern coremio_result f_database_query_execute(s_database *database, s_database_query *database_query, l_database_row f_database_row, void *user_data,
    bool *successfully_rollback);
extern coremio_result f_database_query_execute_list(s_database *database, s_list *database_queries, l_database_row f_database_row, void *user_data,
    bool *successfully_rollback);
extern void f_database_query_free(s_database_query *database_query);
extern void f_database_free(s_database *database);
#endif // COREMIO_DATABASE_H
