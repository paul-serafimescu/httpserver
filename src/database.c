#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <sqlite3.h>
#include <string.h>
#include <stdarg.h>
#include <json_object.h>

#include "database.h"

#define PRINT_ERR(error_msg) { fprintf(stderr, "[SQL error] %s\n", error_msg); }

int build_result(json_t *result, database_t *db,
    const char *query, size_t query_size,
    const char *fmt, va_list args);
static int vprepare(database_t *db, const char *query, int query_size,
    const char *fmt, va_list args);

database_t *create_cursor(const char *file_name)
{
  database_t *db = malloc(sizeof(database_t));
  int rc = sqlite3_open(file_name, &db->db);
  if (rc != SQLITE_OK) {
    fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db->db));
    sqlite3_close(db->db);

    return NULL;
  }
  db->error_message = NULL;
  db->prepared_statement = NULL;
  return db;
}

json_t select_all(database_t *db, const char *table_name)
{
  char *query;
  size_t query_size = asprintf(&query, "SELECT rowid AS id, * FROM %s", table_name);
  json_t result = exec_sql(db, query, query_size + 1, "");
  free(query);
  return result;
}

json_t select_by_id(database_t *db, const char *table_name, const size_t id)
{
  char *query;
  size_t query_size = asprintf(&query, "SELECT rowid AS id, * FROM %s WHERE rowid = ?", table_name);
  json_t result = exec_sql(db, query, query_size + 1, "d", id);
  free(query);
  return result;
}

json_t exec_sql(database_t *db, const char *stmnt, size_t stmnt_size,
    const char *fmt, ...)
{
  va_list args;
  va_start(args, fmt);
  json_t result = NULL;
  if (build_result(&result, db, stmnt, stmnt_size, fmt, args) < 0) {
    PRINT_ERR(db->error_message);
  }
  va_end(args);
  return result;
}

// returns -1 on failure, last added row id on success
int insert_into_table(database_t *db, const char *table_name, const char *fmt, ...)
{
  size_t num_columns = strlen(fmt);
  size_t table_length = strlen(table_name);

  char *query = malloc(table_length + num_columns * 2 + 22);
  char *s = query;
  strcpy(s, "INSERT INTO ");
  s += 12;
  strcpy(s, table_name);
  s += table_length;
  strcpy(s, " VALUES (?");
  s += 10;
  for (size_t i = 1; i < num_columns; i++) {
    strcpy(s, ",?");
    s += 2;
  }
  strcpy(s, ")");

  va_list args;
  va_start(args, fmt);
  int rc = vprepare(db, query, table_length + num_columns * 2 + 22, fmt, args);
  if (rc != SQLITE_OK) {
    db->error_message = (char *)sqlite3_errmsg(db->db);
    PRINT_ERR(db->error_message);
    return -1;
  }
  va_end(args);

  if (sqlite3_step(db->prepared_statement) != SQLITE_DONE) {
    db->error_message = (char *)sqlite3_errmsg(db->db);
    PRINT_ERR(db->error_message);
    sqlite3_finalize(db->prepared_statement);
    free(query);
    return -1;
  }
  sqlite3_finalize(db->prepared_statement);
  free(query);
  return sqlite3_last_insert_rowid(db->db);
}

int delete_by_id(database_t *db, const char *table_name, const size_t id)
{
  char *query;
  int return_value = 0;
  asprintf(&query, "DELETE FROM %s WHERE rowid = %zu", table_name, id);
  if (sqlite3_exec(db->db, query, 0, 0, &db->error_message)) {
    PRINT_ERR(db->error_message);
    return_value = -1;
  }
  free(query);
  return return_value;
}

int update_by_id(database_t *db, const char *table_name, const size_t id, const char *changes, const char *fmt, ...)
{
  char *query;
  asprintf(&query, "UPDATE %s SET %s WHERE rowid = %zu", table_name, changes, id);
  va_list args;
  va_start(args, fmt);
  int rc = vprepare(db, query, -1, fmt, args);
  if (rc != SQLITE_OK) {
    db->error_message = (char *)sqlite3_errmsg(db->db);
    PRINT_ERR(db->error_message);
    return -1;
  }
  va_end(args);
  free(query);

  if (sqlite3_step(db->prepared_statement) != SQLITE_DONE) {
    db->error_message = (char *)sqlite3_errmsg(db->db);
    PRINT_ERR(db->error_message);
    sqlite3_finalize(db->prepared_statement);
    return -1;
  }
  sqlite3_finalize(db->prepared_statement);
  return 0;
}

void destroy_cursor(database_t *db)
{
  if (sqlite3_close_v2(db->db) != SQLITE_OK) {
    db->error_message = (char *)sqlite3_errmsg(db->db);
    PRINT_ERR(db->error_message);
  }
  free(db);
}

json_t get_column_names(database_t *db, const char *table_name)
{
  char *query;
  size_t query_size =
    asprintf(&query, "SELECT name, type FROM pragma_table_info('%s')", table_name);
  json_t column_metadata = exec_sql(db, query, query_size + 1, "");
  free(query);
  return column_metadata;
}

int build_result(json_t *result, database_t *db,
    const char *query, size_t query_size,
    const char *fmt, va_list args)
{
  int rc = vprepare(db, query, query_size, fmt, args);
  if (rc != SQLITE_OK) {
    db->error_message = (char *)sqlite3_errmsg(db->db);
    return -1;
  }
  *result = json_object_new_array();

  int num_columns = sqlite3_column_count(db->prepared_statement);
  char *column_names[num_columns];
  for (int column = 0; column < num_columns; column++) {
    column_names[column] = strdup(sqlite3_column_name(db->prepared_statement, column));
  }

  while((rc = sqlite3_step(db->prepared_statement)) == SQLITE_ROW) {
    json_t row = json_object_new_object();
    for (int i = 0; i < num_columns; i++) {
      int type = sqlite3_column_type(db->prepared_statement, i);
      json_t entry = NULL;
      switch (type) {
        case SQLITE_TEXT:
          entry = json_object_new_string((const char *)sqlite3_column_text(db->prepared_statement, i));
          break;
        case SQLITE_INTEGER:
          entry = json_object_new_int(sqlite3_column_int(db->prepared_statement, i));
          break;
        case SQLITE_FLOAT:
          entry = json_object_new_double(sqlite3_column_double(db->prepared_statement, i));
          break;
      }
      json_object_object_add(row, column_names[i], entry);
    }
    json_object_array_add(*result, row);
  }

  for (int column = 0; column < num_columns; column++) {
     free(column_names[column]);
  }
  sqlite3_finalize(db->prepared_statement);
  return 0;
}

int vprepare(database_t *db, const char *query, int query_size,
    const char *fmt, va_list args)
{
  int rc = sqlite3_prepare_v2(db->db, query, query_size, &db->prepared_statement, NULL);
  if (rc != SQLITE_OK) {
    return rc;
  }
  int argnum = 1;
  for (size_t columnnum = 0; fmt[columnnum]; columnnum++) {
    char type = fmt[columnnum];
    switch (type) {
      case 'd':
        rc = sqlite3_bind_int(db->prepared_statement, argnum, va_arg(args, int));
        break;
      case 'f':
        rc = sqlite3_bind_double(db->prepared_statement, argnum, va_arg(args, double));
        break;
      case 's':
        ;
        char *t = va_arg(args, char *);
        size_t len = va_arg(args, size_t);
        if (len == 0 && t[0]) {
          len = strlen(t);
        }
        rc = sqlite3_bind_text(db->prepared_statement, argnum,
            t, len, SQLITE_TRANSIENT);
        break;
      default:
        rc = sqlite3_bind_null(db->prepared_statement, argnum);
        break;
    }
    if (rc != SQLITE_OK) {
      return rc;
    }
    argnum++;
  }
  return SQLITE_OK;
}
