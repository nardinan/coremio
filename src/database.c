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
#include "../include/coremio/database.h"
#include <_strings.h>
#include <pthread.h>
#include <sqlite3.h>
#include <stdbool.h>
#include "../include/coremio/tokens.h"
d_result_define(SHIT_DATABASE_INVALID_PARAMETER, 1, "Failure: impossible to use the given parameters to configure the database");
d_result_define(SHIT_DATABASE_INVALID_OPEN, 2, "Failure: impossible to open the database");
d_result_define(SHIT_DATABASE_INVALID_QUERY, 3, "Failure: query in the database has failed");
coremio_result f_database_initialize(s_database *database, const char *path, const int flags, const time_t busy_timeout_milliseconds) {
  coremio_result result = NOICE;
  memset(database, 0, sizeof(s_database));
  if ((path) && (busy_timeout_milliseconds > 0)) {
    if ((sqlite3_open_v2(path, &(database->database), flags, NULL)) == SQLITE_OK) {
      if (sqlite3_busy_timeout(database->database, busy_timeout_milliseconds) == SQLITE_OK) {
        pthread_mutexattr_t database_lock_attributes;
        if (pthread_mutexattr_init(&database_lock_attributes) == 0) {
          if (pthread_mutexattr_settype(&database_lock_attributes, PTHREAD_MUTEX_RECURSIVE) == 0) {
            if (pthread_mutex_init(&(database->database_lock), &database_lock_attributes) != 0)
              result = SHIT_NO_MEMORY;
          } else
            result = SHIT_INVALID_PARAMETERS;
          pthread_mutexattr_destroy(&database_lock_attributes);
        } else
          result = SHIT_NO_MEMORY;
      } else
        result = SHIT_DATABASE_INVALID_PARAMETER;
      if (result != NOICE) {
        sqlite3_close(database->database);
        database->database = NULL;
      }
    } else
      result = SHIT_DATABASE_INVALID_OPEN;
  } else
    result = SHIT_INVALID_PARAMETERS;
  return result;
}
coremio_result f_database_run_raw_command(s_database *database, const char *command) {
  coremio_result result = NOICE;
  if (database->database) {
    if (sqlite3_exec(database->database, command, NULL, NULL, NULL) != SQLITE_OK)
      result = SHIT_DATABASE_INVALID_PARAMETER;
  } else
    result = SHIT_NOT_INITIALIZED;
  return result;
}
coremio_result f_database_query_prepare_args(s_database_query *database_query, const char *query, const char *types, va_list parameters) {
  coremio_result result = NOICE;
  memset(database_query, 0, sizeof(s_database_query));
  if (query) {
    size_t query_length = strlen(query);
    database_query->values_count = ((types) ? strlen(types) : 0);
    if ((database_query->query = (char *) d_malloc(query_length + 1))) {
      s_database_query_value *query_value;
      strncpy(database_query->query, query, query_length);
      database_query->query[query_length] = 0;
      if (database_query->values_count > 0) {
        t_token token;
        for (size_t index_parameter = 0; ((result == NOICE) && (index_parameter < database_query->values_count)); ++index_parameter) {
          s_database_query_value *query_value;
          if ((query_value = (s_database_query_value *) d_malloc(sizeof(s_database_query_value)))) {
            memset(query_value, 0, sizeof(s_database_query_value));
            switch (types[index_parameter]) {
              case 's': {
                const char *raw_value = va_arg(parameters, const char *);
                if (raw_value) {
                  query_value->token = f_tokens_new_token_char(raw_value, false);
                  query_value->token_size = (strlen(raw_value) + 1);
                }
                break;
              }
              case 'i': {
                query_value->token = f_tokens_new_token_int(va_arg(parameters, int));
                query_value->token_size = sizeof(int);
                break;
              }
              case 'd': {
                double raw_value = va_arg(parameters, double);
                if (raw_value == raw_value) {
                  query_value->token = f_tokens_new_token_double(raw_value);
                  query_value->token_size = sizeof(double);
                } /* NAN gets converted into NULL directly by sqlite3 so, this makes sense */
                break;
              }
              case 'n': {
                break;
              }
              default: {
                result = SHIT_INVALID_PARAMETERS;
              }
            }
            if (result == NOICE)
              f_list_append(&(database_query->values), (s_list_node *) query_value, e_list_insert_tail);
            else
              d_free(query_value);
          } else
            result = SHIT_NO_MEMORY;
        }
      }
      if (result != NOICE)
        f_database_query_free(database_query);
    } else
      result = SHIT_NO_MEMORY;
  } else
    result = SHIT_INVALID_PARAMETERS;
  return result;
}
coremio_result f_database_query_prepare(s_database_query *database_query, const char *query, const char *types, ...) {
  coremio_result result;
  va_list parameters;
  va_start(parameters, types);
  {
    result = f_database_query_prepare_args(database_query, query, types, parameters);
  }
  va_end(parameters);
  return result;
}
static coremio_result p_database_query_execute_index(s_database *database, s_database_query *database_query, l_database_row f_database_row, void *user_data,
    size_t query_index) {
  coremio_result result = NOICE;
  sqlite3_stmt *query_details_statement = NULL;
  if (sqlite3_prepare_v2(database->database, database_query->query, -1, &query_details_statement, NULL) == SQLITE_OK) {
    if ((size_t) sqlite3_bind_parameter_count(query_details_statement) == database_query->values_count) {
      int query_status = SQLITE_OK;
      s_database_query_value *query_value;
      size_t index_value = 1;
      for (query_value = (s_database_query_value *) database_query->values.head; (query_value) && (query_status == SQLITE_OK);
          query_value = (s_database_query_value *) (query_value->head.next)) {
        if (query_value->token_size == 0) {
          query_status = sqlite3_bind_null(query_details_statement, (index_value++));
        } else if (!d_boxed_nan_is_boxed_nan(query_value->token)) {
          query_status = sqlite3_bind_double(query_details_statement, (index_value++), query_value->token);
        } else {
          switch (d_boxed_nan_get_signature(query_value->token)) {
            case d_boxed_nan_special_character_symbol: {
              query_status = sqlite3_bind_double(query_details_statement, (index_value++), NAN);
              break;
            }
            case d_boxed_nan_pointer_string_signature:
            case d_boxed_nan_pointer_quoted_string_signature: {
              query_status = sqlite3_bind_text(query_details_statement, (index_value++), d_boxed_nan_get_pointer(query_value->token), -1, SQLITE_TRANSIENT);
              break;
            }
            case d_boxed_nan_embedded_string_signature: {
              char embedded_string[d_boxed_nan_available_bytes] = {0};
              f_boxed_nan_get_embedded_string(query_value->token, embedded_string);
              query_status = sqlite3_bind_text(query_details_statement, (index_value++), embedded_string, -1, SQLITE_TRANSIENT);
              break;
            }
            case d_boxed_nan_int_signature: {
              query_status = sqlite3_bind_int(query_details_statement, (index_value++), d_boxed_nan_get_int(query_value->token));
              break;
            }
            default: {
              query_status = SQLITE_ERROR_UNABLE;
            }
          }
        }
      }
      if (query_status == SQLITE_OK) {
        const int columns_count = sqlite3_column_count(query_details_statement);
        if ((columns_count > 0) && (database->cache_row.columns_count < columns_count)) {
          const char **new_cache_keys = NULL, **new_cache_values = NULL;
          if ((!(new_cache_keys = (const char **) d_realloc(database->cache_row.keys, (columns_count * sizeof(char *))))) ||
              (!(new_cache_values = (const char **) d_realloc(database->cache_row.values, (columns_count * sizeof(char *)))))) {
            if (new_cache_keys)
              d_free(new_cache_keys);
            else if (database->cache_row.keys)
              d_free(database->cache_row.keys);
            database->cache_row.keys = NULL;
            if (new_cache_values)
              d_free(new_cache_values);
            else if (database->cache_row.values)
              d_free(database->cache_row.values);
            database->cache_row.values = NULL;
            database->cache_row.columns_count = 0;
            result = SHIT_NO_MEMORY;
          } else {
            database->cache_row.keys = new_cache_keys;
            database->cache_row.values = new_cache_values;
            database->cache_row.columns_count = columns_count;
          }
        }
        if (result == NOICE) {
          while ((query_status = sqlite3_step(query_details_statement)) == SQLITE_ROW)
            if (f_database_row) {
              for (size_t index_column = 0; index_column < columns_count; ++index_column) {
                database->cache_row.keys[index_column] = sqlite3_column_name(query_details_statement, index_column);
                database->cache_row.values[index_column] = (const char *) sqlite3_column_text(query_details_statement, index_column);
              }
              f_database_row(database, database_query, query_index, columns_count, database->cache_row.keys, database->cache_row.values, user_data);
            }
          if (query_status != SQLITE_DONE)
            result = SHIT_DATABASE_INVALID_QUERY;
        } else
          result = SHIT_NO_MEMORY;
      } else
        result = SHIT_DATABASE_INVALID_PARAMETER;
    } else
      result = SHIT_DATABASE_INVALID_PARAMETER;
    sqlite3_finalize(query_details_statement);
  } else
    result = SHIT_DATABASE_INVALID_PARAMETER;
  return result;
}
coremio_result f_database_query_execute(s_database *database, s_database_query *database_query, l_database_row f_database_row, void *user_data,
    bool *successfully_rollback) {
  coremio_result result = NOICE;
  pthread_mutex_lock(&(database->database_lock));
  {
    if ((result = f_database_run_raw_command(database, "BEGIN IMMEDIATE;")) == NOICE) {
      if ((result = p_database_query_execute_index(database, database_query, f_database_row, user_data, 0)) == NOICE)
        result = f_database_run_raw_command(database, "COMMIT;");
      else {
        if ((f_database_run_raw_command(database, "ROLLBACK;") == NOICE) && (successfully_rollback))
          *successfully_rollback = true;
        else if (successfully_rollback)
          *successfully_rollback = false;
      }
    }
  }
  pthread_mutex_unlock(&(database->database_lock));
  return result;
}
coremio_result f_database_query_execute_list(s_database *database, s_list *database_queries, l_database_row f_database_row, void *user_data,
    bool *successfully_rollback) {
  coremio_result result = NOICE;
  pthread_mutex_lock(&(database->database_lock));
  {
    if ((result = f_database_run_raw_command(database, "BEGIN IMMEDIATE;")) == NOICE) {
      s_database_query *current_database_query;
      size_t query_index = 0;
      for (current_database_query = (s_database_query *) database_queries->head; (current_database_query) && (result == NOICE);
          current_database_query = (s_database_query *) current_database_query->head.next) {
        result = p_database_query_execute_index(database, current_database_query, f_database_row, user_data, (query_index++));
      }
      if (result == NOICE)
        result = f_database_run_raw_command(database, "COMMIT;");
      else {
        if ((f_database_run_raw_command(database, "ROLLBACK;") == NOICE) && (successfully_rollback))
          *successfully_rollback = true;
        else if (successfully_rollback)
          *successfully_rollback = false;
      }
    }
  }
  pthread_mutex_unlock(&(database->database_lock));
  return result;
}
void f_database_query_free(s_database_query *database_query) {
  s_database_query_value *query_value;
  if (database_query->query)
    d_free(database_query->query);
  while ((query_value = (s_database_query_value *) database_query->values.head)) {
    f_list_remove_from_owner(database_query->values.head);
    if (query_value->token_size > 0)
      f_tokens_free_token_content(query_value->token);
    d_free(query_value);
  }
}
void f_database_free(s_database *database) {
  if (database->database) {
    pthread_mutex_lock(&(database->database_lock));
    {
      sqlite3_close(database->database);
      database->database = NULL;
      if (database->cache_row.keys)
        d_free(database->cache_row.keys);
      if (database->cache_row.values)
        d_free(database->cache_row.values);
      database->cache_row.columns_count = 0;
    }
    pthread_mutex_unlock(&(database->database_lock));
    pthread_mutex_destroy(&(database->database_lock));
  }
}
