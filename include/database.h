#ifndef DATABASE_H
#define DATABASE_H

#include <sqlite3.h>

typedef union db_entry_t { char *t; int i; double d; void *v; } db_entry_t;

typedef struct {
  enum {
    INTEGER = SQLITE_INTEGER,
    REAL = SQLITE_FLOAT,
    TEXT = SQLITE_TEXT,
    BLOB = SQLITE_BLOB,
    NULL_VALUE = SQLITE_NULL
  } type;
  db_entry_t *entries;
  size_t num_rows;
  size_t capacity;
} column_t;

typedef struct {
  column_t *columns;
  size_t num_cols;
} sql_result_t;

typedef struct {
  sqlite3 *db;
  sqlite3_stmt *prepared_statement;
  char *error_message;
} database_t;

database_t *create_cursor(const char *file_name);
sql_result_t *init_result();
int build_result(sql_result_t *result, database_t *db, const char *query, size_t query_size);
sql_result_t *select_all(database_t *db, const char *table_name);
void destroy_result(sql_result_t *result);
void destroy_cursor(database_t *db);
void print_result(sql_result_t *result);

#endif
