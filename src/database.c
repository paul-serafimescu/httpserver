#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <sqlite3.h>
#include <string.h>
#include <stdarg.h>
#include "database.h"

#define HANDLE_QUERYSET_ERR do { fprintf(stderr, "Error building queryset result.\n"); } while (0)

static void resize_result(sql_result_t *result);
static char *str_copy_from(const char *src);

database_t *create_cursor(const char *file_name)
{
  sqlite3 *sqlite;
  database_t *db = (database_t *)malloc(sizeof(database_t));
  int rc = sqlite3_open(file_name, &sqlite);
  if (rc != SQLITE_OK) {
    fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(sqlite));
    sqlite3_close(db->db);

    return NULL;
  }
  db->db = sqlite;
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
  sql_result_t *result = init_result();
  char *query;
  size_t query_size = asprintf(&query, "SELECT * FROM %s", table_name);
  if (build_result(result, db, query, query_size) < 0) {
    HANDLE_QUERYSET_ERR;
  }
  free(query);
  return result;
}

sql_result_t *select_by_id(database_t *db, const char *table_name, const size_t id)
{
  sql_result_t *result = init_result();
  char *query;
  size_t query_size = asprintf(&query, "SELECT * FROM %s WHERE rowid = %zu", table_name, id);
  if (build_result(result, db, query, query_size) < 0) {
    HANDLE_QUERYSET_ERR;
  }
  free(query);
  return result;
}

// format should be something like (%s, %d, %s)
// returns -1 on failure, last added row id on success
int insert_into_table(database_t *db, const char *table_name, const char *format, ...)
{
  char *query, *frmt;
  asprintf(&frmt, "INSERT INTO %s VALUES %s", table_name, format);
  va_list args;
  va_start(args, frmt);
  vasprintf(&query, frmt, args);
  if (sqlite3_exec(db->db, query, 0, 0, &db->error_message) != SQLITE_OK) {
    fprintf(stderr, "SQL error: %s", db->error_message);
    return -1;
  }
  free(frmt);
  free(query);
  return sqlite3_last_insert_rowid(db->db);
}

sql_result_t *exec_sql(database_t *db, const char *stmnt)
{
  sql_result_t *result = init_result();
  if (build_result(result, db, stmnt, strlen(stmnt) + 1) < 0) {
    HANDLE_QUERYSET_ERR;
  }
  return result;
}

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
    printf("whoops, SQLite3 broke!\n");
  }
  free(db->error_message);
  free(db);
}

static void resize_result(sql_result_t *result)
{
  if (result->num_rows == result->capacity) {
    result->capacity *= 2;
    result->rows = (row_t *)realloc(result->rows, sizeof(row_t) * result->capacity);
  }
  // printf("num rows: %zu, capacity: %zu\n", result->num_rows, result->capacity);
}

static char *str_copy_from(const char *src)
{
  size_t src_length = strlen(src) + 1;
  char *dest = (char *)malloc(src_length);
  strncpy(dest, src, src_length);
  return dest;
}

sql_result_t *get_column_names(database_t *db, const char *table_name)
{
  char *query;
  asprintf(&query, "SELECT name, type FROM pragma_table_info('%s')", table_name);
  sql_result_t *column_metadata = exec_sql(db, query);
  free(query);
  return column_metadata;
}

int build_result(sql_result_t *result, database_t *db, const char *query, size_t query_size)
{
  int rc = sqlite3_prepare_v2(db->db, query, query_size, &db->prepared_statement, NULL);
  if (rc != SQLITE_OK) {
    fprintf(stderr, "Error preparing query.\n");
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
        result->column_info[i].name = str_copy_from(sqlite3_column_name(db->prepared_statement, i));
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
        case SQLITE_TEXT:;
          entry.t = str_copy_from((char *)sqlite3_column_text(db->prepared_statement, i));
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
