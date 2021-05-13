#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <sqlite3.h>
#include <string.h>
#include <stdarg.h>
#include "database.h"

#define PRINT_ERR(error_msg) { fprintf(stderr, "[SQL error] %s\n", error_msg); }

static void resize_result(sql_result_t *result);

database_t *create_cursor(const char *file_name)
{
  database_t *db = (database_t *)malloc(sizeof(database_t));
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

sql_result_t *init_result()
{
  sql_result_t *result = (sql_result_t *)malloc(sizeof(sql_result_t));
  result->num_rows = 0;
  result->capacity = 1;
  result->column_info = NULL;
  result->rows = (row_t *)malloc(result->capacity * sizeof(row_t));
  return result;
}

sql_result_t *select_all(database_t *db, const char *table_name)
{
  char *query;
  size_t query_size = asprintf(&query, "SELECT * FROM %s", table_name);
  sql_result_t *result = exec_sql(db, query, query_size + 1);
  free(query);
  return result;
}

sql_result_t *select_by_id(database_t *db, const char *table_name, const size_t id)
{
  char *query;
  size_t query_size = asprintf(&query, "SELECT * FROM %s WHERE rowid = %zu", table_name, id);
  sql_result_t *result = exec_sql(db, query, query_size + 1);
  free(query);
  return result;
}

// returns -1 on failure, last added row id on success
int insert_into_table(database_t *db, const char *table_name, ...)
{
  sql_result_t *columns = get_column_names(db, table_name);
  size_t table_length = strlen(table_name);

  char *query = (char *)malloc(table_length + columns->num_rows * 2 + 22);
  char *s = query;
  strcpy(s, "INSERT INTO ");
  s += 12;
  strcpy(s, table_name);
  s += table_length;
  strcpy(s, " VALUES (?");
  s += 10;
  for (size_t i = 1; i < columns->num_rows; i++) {
    strcpy(s, ",?");
    s += 2;
  }
  strcpy(s, ")");

  sqlite3_prepare_v2(db->db,
      query, table_length + columns->num_rows * 2 + 22,
      &db->prepared_statement, NULL);

  va_list args;
  va_start(args, table_name);
  int argnum = 1;
  for (size_t columnnum = 0; columnnum < columns->num_rows; columnnum++) {
    char *typename = columns->rows[columnnum][1].t;
    if (!strcmp(typename, "TEXT")) {
      char *t = va_arg(args, char *);
      size_t len = va_arg(args, size_t);
      if (len == 0 && t[0]) {
        len = strlen(t);
      }
      sqlite3_bind_text(db->prepared_statement, argnum,
          t, len, SQLITE_TRANSIENT);
    } else if (!strcmp(typename, "REAL")) {
      sqlite3_bind_double(db->prepared_statement, argnum, va_arg(args, double));
    } else if (!strcmp(typename, "INTEGER")) {
      sqlite3_bind_int(db->prepared_statement, argnum, va_arg(args, int));
    } else {
      sqlite3_bind_null(db->prepared_statement, argnum);
    }
    argnum++;
  }
  va_end(args);

  if (sqlite3_step(db->prepared_statement) != SQLITE_DONE) {
    db->error_message = (char *)sqlite3_errmsg(db->db);
    PRINT_ERR(db->error_message);
    sqlite3_finalize(db->prepared_statement);
    destroy_result(columns);
    free(query);
    return -1;
  }
  sqlite3_finalize(db->prepared_statement);
  destroy_result(columns);
  free(query);
  return sqlite3_last_insert_rowid(db->db);
}

sql_result_t *exec_sql(database_t *db, const char *stmnt, size_t stmnt_size)
{
  if (stmnt_size == 0) {
    stmnt_size = strlen(stmnt) + 1;
  }
  sql_result_t *result = init_result();
  if (build_result(result, db, stmnt, stmnt_size) < 0) {
    PRINT_ERR(db->error_message);
  }
  return result;
}

int delete_by_id(database_t *db, const char *table_name, const size_t id)
{
  char *query;
  signed char return_value = 0;
  asprintf(&query, "DELETE FROM %s WHERE rowid = %zu", table_name, id);
  if (sqlite3_exec(db->db, query, 0, 0, &db->error_message)) {
    PRINT_ERR(db->error_message);
    return_value = -1;
  }
  free(query);
  return return_value;
}

/* int update_by_id(database_t *db, const char *table_name, const size_t id)
{
  char *query;
  asprintf(&query, "UPDATE %s SET nyaa~ WHERE rowid = %zu", table_name, id);
} */

void print_result(sql_result_t *result)
{
  size_t i, j;
  for (i = 0; i < result->num_cols; i++) {
    printf("%s ", result->column_info[i].name);
  }
  printf("\n");
  for (i = 0 ; i < result->num_rows; i++) {
    for (j = 0; j < result->num_cols; j++) {
      switch (result->column_info[j].type) {
        case TEXT:
          printf("%s ", result->rows[i][j].t);
          break;
        case REAL:
          printf("%f ", result->rows[i][j].d);
          break;
        case INTEGER:
          printf("%d ", result->rows[i][j].i);
          break;
        case NULL_VALUE:
          printf("%s ", "NULL");
          break;
        default:
          break;
      }
    }
    printf("\n");
  }
}

void destroy_result(sql_result_t *result)
{
  size_t i;
  for (size_t j = 0; j < result->num_cols; j++) {
    if (result->column_info[j].type == TEXT) {
      for (i = 0; i < result->num_rows; i++) {
        free(result->rows[i][j].t);
      }
    }
  }
  for (i = 0; i < result->num_rows; i++) {
    free(result->rows[i]);
  }
  for (i = 0; i < result->num_cols; i++) {
    free(result->column_info[i].name);
  }
  free(result->column_info);
  free(result->rows);
  free(result);
}

void destroy_cursor(database_t *db)
{
  if (sqlite3_close_v2(db->db) != SQLITE_OK) {
    db->error_message = (char *)sqlite3_errmsg(db->db);
    PRINT_ERR(db->error_message);
  }
  free(db);
}

static void resize_result(sql_result_t *result)
{
  if (result->num_rows == result->capacity) {
    result->capacity *= 2;
    result->rows = (row_t *)realloc(result->rows, sizeof(row_t) * result->capacity);
  }
}

sql_result_t *get_column_names(database_t *db, const char *table_name)
{
  char *query;
  size_t query_size =
    asprintf(&query, "SELECT name, type FROM pragma_table_info('%s')", table_name);
  sql_result_t *column_metadata = exec_sql(db, query, query_size + 1);
  free(query);
  return column_metadata;
}

int build_result(sql_result_t *result, database_t *db, const char *query, size_t query_size)
{
  int rc = sqlite3_prepare_v2(db->db, query, query_size, &db->prepared_statement, NULL);
  if (rc != SQLITE_OK) {
    db->error_message = (char *)sqlite3_errmsg(db->db);
    return -1;
  }
  int i, row, num_columns = sqlite3_column_count(db->prepared_statement);
  result->num_cols = num_columns;
  result->column_info = (column_t *)malloc(sizeof(column_t) * num_columns);
  for (row = 0; (rc = sqlite3_step(db->prepared_statement)) == SQLITE_ROW; row++) {
    resize_result(result);
    result->rows[row] = (db_entry_t *)malloc(num_columns * sizeof(db_entry_t));
    for (i = 0; i < num_columns; i++) {
      int type = sqlite3_column_type(db->prepared_statement, i);
      if (!row) {
        result->column_info[i].name = strdup(sqlite3_column_name(db->prepared_statement, i));
        switch (type) {
          case SQLITE_TEXT:
            result->column_info[i].type = TEXT;
            break;
          case SQLITE_INTEGER:
            result->column_info[i].type = INTEGER;
            break;
          case SQLITE_FLOAT:
            result->column_info[i].type = REAL;
            break;
          default:
            result->column_info[i].type = NULL_VALUE;
            break;
        }
      }
      db_entry_t entry;
      switch (type) {
        case SQLITE_TEXT:
          entry.t = strdup((char *)sqlite3_column_text(db->prepared_statement, i));
          break;
        case SQLITE_INTEGER:
          entry.i = sqlite3_column_int(db->prepared_statement, i);
          break;
        case SQLITE_FLOAT:
          entry.d = sqlite3_column_double(db->prepared_statement, i);
          break;
        default:
          entry.v = NULL;
          break;
      }
      result->rows[row][i] = entry;
    }
    result->num_rows++;
  }
  sqlite3_finalize(db->prepared_statement);
  return 0;
}
