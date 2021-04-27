#include <stdlib.h>
#include <stdio.h>
#include <sqlite3.h>
#include <string.h>
#include "database.h"

// idek

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

sql_result_t *select_all(database_t *db, const char *table_name)
{
  sql_result_t *result = (sql_result_t *)malloc(sizeof(sql_result_t));
  result->num_cols = 0;

  size_t query_size = strlen(table_name) + 15;
  char *query = (char *)malloc(query_size);
  sprintf(query, "SELECT * FROM %s", table_name);
  int rc = sqlite3_prepare_v2(db->db, query, query_size, &db->prepared_statement, NULL);
  free(query);
  if (rc != SQLITE_OK) {
    fprintf(stderr, "Error preparing query.\n");
  }
  int row, num_columns = sqlite3_column_count(db->prepared_statement);
  result->columns = (column_t *)malloc(sizeof(column_t) * num_columns);
  result->num_cols = num_columns;
  int initializer;
  for (initializer = 0; initializer < num_columns; initializer++) {
    result->columns[initializer].num_rows = 0;
    result->columns[initializer].max_rows = 1;
    result->columns[initializer].values = (db_entry_t *)malloc(sizeof(db_entry_t));
  }
  for (row = 0; (rc = sqlite3_step(db->prepared_statement)) == SQLITE_ROW; row++) {
    int i;
    for (i = 0; i < num_columns; i++) {
      if (result->columns[i].num_rows == result->columns[i].max_rows) {
        result->columns[i].max_rows *= 2;
        result->columns[i].values = (db_entry_t *)realloc(result->columns[i].values, sizeof(db_entry_t) * result->columns[i].max_rows);
      }
      int type = sqlite3_column_type(db->prepared_statement, i);
      db_entry_t entry;
      switch (type) {
        case SQLITE_TEXT:;
          char *cell = (char *)sqlite3_column_text(db->prepared_statement, i);
          size_t value_length = strlen(cell) + 1;
          entry.t = (char *)malloc(value_length);
          strncpy(entry.t, cell, value_length);
          result->columns[i].values[row] = entry;
          result->columns[i].type = TEXT;
          break;
        case SQLITE_INTEGER:
          entry.i = sqlite3_column_int(db->prepared_statement, i);
          result->columns[i].values[row] = entry;
          result->columns[i].type = INTEGER;
          break;
        case SQLITE_FLOAT:
          entry.d = sqlite3_column_double(db->prepared_statement, i);
          result->columns[i].values[row] = entry;
          result->columns[i].type = REAL;
          break;
        default:
          entry.v = NULL;
          result->columns[i].values[row] = entry;
          result->columns[i].type = NULL_VALUE;
          break;
      }
      result->columns[i].num_rows++;
    }
  }
  sqlite3_finalize(db->prepared_statement);
  return result;
}

void print_result(sql_result_t *result)
{
  size_t i;
  for (i = 0 ; i < result->num_cols; i++) {
    size_t j;
    for (j = 0; j < result->columns[i].num_rows; j++) {
      switch (result->columns[i].type) {
        case TEXT:
          printf("%s\n", result->columns[i].values[j].t);
          break;
        case REAL:
          printf("%f\n", result->columns[i].values[j].d);
          break;
        case INTEGER:
          printf("%d\n", result->columns[i].values[j].i);
          break;
        case NULL_VALUE:
          printf("%s\n", "NULL");
          break;
        default:
          break;
      }
    }
  }
}

void destroy_result(sql_result_t *result)
{
  size_t i;
  for (i = 0; i < result->num_cols; i++) {
    size_t j;
    if (result->columns[i].type == TEXT)
      for (j = 0; j < result->columns[i].num_rows; j++)
        free(result->columns[i].values[j].t);
    free(result->columns[i].values);
  }
  free(result->columns);
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
