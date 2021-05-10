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
  char *name;
} column_t;

typedef db_entry_t *row_t;

typedef struct {
  row_t *rows;
  column_t *column_info;
  size_t num_cols;
  size_t num_rows;
  size_t capacity;
} sql_result_t;

typedef struct {
  sqlite3 *db;
  sqlite3_stmt *prepared_statement;
  char *error_message;
} database_t;

// initializers
database_t *create_cursor(const char *file_name);
sql_result_t *init_result();
int build_result(sql_result_t *result, database_t *db, const char *query, size_t query_size);

// built ins
sql_result_t *select_all(database_t *db, const char *table_name);
sql_result_t *select_by_id(database_t *db, const char *table_name, const size_t id);
sql_result_t *exec_sql(database_t *db, const char *stmnt, size_t stmnt_size);
sql_result_t *get_column_names(database_t *db, const char *table_name);
int insert_into_table(database_t *db, const char *table_name, ...);
// deallocators
void destroy_result(sql_result_t *result);
void destroy_cursor(database_t *db);
void print_result(sql_result_t *result);

#endif
