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



// INITIALIZERS

/*
 * Creates and initializes cursor to specified in-memory SQLite3 database.
 * Returns structure containing connection, error handling, and prepared statement structure.
 */
database_t *create_cursor(const char *file_name);

/*
 * Creates and initializes SQL result structure.
 * Returns result structure.
 */
sql_result_t *init_result();

/*
 * Helper function for built-ins.
 * Populates SQL result structure with row-by-row data after executing a prepared statement.
 * Returns -1 on failure, 0 on success.
 */
int build_result(sql_result_t *result, database_t *db, const char *query, size_t query_size);



// BUILT-INS

/*
 * Selects all rows and columns of table from database.
 * Returns populated SQL result structure on success, NULL on failure.
 */
sql_result_t *select_all(database_t *db, const char *table_name);

/*
 * Selects all columns of a single row with primary key 'id'.
 * Returns populated SQL result structure on success, NULL on failure.
 */
sql_result_t *select_by_id(database_t *db, const char *table_name, const size_t id);

/*
 * Executes arbitrary SQL.
 * Returns populated SQL result structure on success, NULL on failure.
 */
sql_result_t *exec_sql(database_t *db, const char *stmnt, size_t stmnt_size);

/*
 * Executes query on PRAGMA to get table columns and types.
 * Returns populated SQL result structure on success, NULL on failure.
 */
sql_result_t *get_column_names(database_t *db, const char *table_name);

/*
 * Inserts values into table.
 * TEXT inserts must be followed by a string length. If unknown, pass 0.
 * Returns -1 on failure, last added row primary key on success.
 */
int insert_into_table(database_t *db, const char *table_name, ...);

/*
 * Deletes row specified by 'id'.
 * Returns -1 on failure, 0 on success.
 */
int delete_by_id(database_t *db, const char *table_name, const size_t id);

/*
 * Has not been implemented yet.
 */
int update_by_id(database_t *db, const char *table_name, const size_t id);



// DEALLOCATORS

/*
 * Deallocates SQL result structure.
 */
void destroy_result(sql_result_t *result);

/*
 * Deallocates database cursor structure.
 */
void destroy_cursor(database_t *db);



// DEBUG

/*
 * Print SQL result structure to console.
 */
void print_result(sql_result_t *result);

#endif
