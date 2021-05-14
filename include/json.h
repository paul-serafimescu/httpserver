#ifndef JSON_H
#define JSON_H

#include "database.h"

/*
 * Converts the result of a database operation to JSON format.
 * Returns JSON string.
 */
char *json_stringify(sql_result_t *result);

#endif
