#ifndef DATABASE_H
#define DATABASE_H

#include <sqlite3.h>
#include <json_object.h>

typedef struct {
  sqlite3 *db;
  sqlite3_stmt *prepared_statement;
  char *error_message;
} database_t;

typedef struct json_object *json_t;



// INITIALIZERS

/*
 * Creates and initializes cursor to specified in-memory SQLite3 database.
 * Returns structure containing connection, error handling, and prepared statement structure.
 */
database_t *create_cursor(const char *file_name);



// BUILT-INS

/*
 * Selects all rows and columns of table from database.
 * Returns json array of objects on success, NULL on failure.
 */
json_t select_all(database_t *db, const char *table_name);

/*
 * Selects all columns of a single row with primary key 'id'.
 * Returns json array of objects on success, NULL on failure.
 */
json_t select_by_id(database_t *db, const char *table_name, const size_t id);

/*
 * Executes arbitrary SQL.
 * fmt contains a sequence of characters, one for each '?' in the statement.
 * These characters are chosen from 'dfsbn',
 * representing an integer, real number, text, blob, or null, respectively.
 * TEXT inserts must be followed by a string length. If unknown, pass 0.
 * Returns json array of objects on success, NULL on failure.
 */
json_t exec_sql(database_t *db, const char *stmnt, size_t stmnt_size,
    const char *fmt, ...);

/*
 * Executes query on PRAGMA to get table columns and types.
 * Returns json array of objects on success, NULL on failure.
 */
json_t get_column_names(database_t *db, const char *table_name);

/*
 * Inserts values into table.
 * fmt contains a sequence of characters, one for each column in the table.
 * These characters are chosen from 'dfsbn',
 * representing an integer, real number, text, blob, or null, respectively.
 * TEXT inserts must be followed by a string length. If unknown, pass 0.
 * Returns -1 on failure, last added row primary key on success.
 */
int insert_into_table(database_t *db, const char *table_name, const char *fmt, ...);

/*
 * Deletes row specified by 'id'.
 * Returns -1 on failure, 0 on success.
 */
int delete_by_id(database_t *db, const char *table_name, const size_t id);

/*
 * Updates row specified by 'id' with the specified transformation.
 * The 'changes' field specifies how to update the row,
 * as in the "SET" clause in the update command.
 * fmt contains a sequence of characters, one for each question mark
 * in the 'changes' field.
 * These characters are chosen from 'dfsbn',
 * representing an integer, real number, text, blob, or null, respectively.
 * TEXT inserts must be followed by a string length. If unknown, pass 0.
 * Returns -1 on failure, 0 on success.
 */
int update_by_id(database_t *db, const char *table_name, const size_t id,
    const char *changes,
    const char *fmt, ...);



// DEALLOCATORS

/*
 * Deallocates database cursor structure.
 */
void destroy_cursor(database_t *db);



#endif
