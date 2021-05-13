#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "database.h"

static void resize(bool condition, char *json_row, size_t *capacity)
{
  if (condition) {
    *capacity *= 2;
    json_row = (char *)realloc(json_row, *capacity);
  }
}

static char *serialize_row(column_t *fields, row_t row, size_t field_count)
{
  size_t i, size = 2, capacity = 7 * field_count, add_mem;
  char *formatted_json = (char *)malloc(capacity);
  strncpy(formatted_json, "{", 2);
  for (i = 0; i < field_count; i++) {
    size_t field_name_len = strlen(fields[i].name);
    resize(capacity <= size + 3 + field_name_len, formatted_json, &capacity);
    strncat(formatted_json, "\"", 2);
    strncat(formatted_json, fields[i].name, field_name_len);
    strncat(formatted_json, "\":", 3);
    size += 3 + field_name_len;
    char *digits;
    switch (fields[i].type) {
      case INTEGER:
        add_mem = asprintf(&digits, "%d", row[i].i);
        resize(capacity <= size + add_mem + 1, formatted_json, &capacity);
        strncat(formatted_json, digits, add_mem + 1);
        break;
      case REAL:
        add_mem = asprintf(&digits, "%f", row[i].d);
        resize(capacity <= size + add_mem + 1, formatted_json, &capacity);
        strncat(formatted_json, digits, add_mem + 1);
        break;
      case TEXT:
        add_mem = asprintf(&digits, "\"%s\"", row[i].t);
        resize(capacity <= size + add_mem + 1, formatted_json, &capacity);
        strncat(formatted_json, digits, add_mem + 1);
        break;
      default:
        break; // TODO: BLOB and NULL
    }
    size += add_mem;
    free(digits);
    if (i != field_count - 1) {
      strncat(formatted_json, ",", 3);
      size++;
    }
  }
  resize(capacity <= size + 1, formatted_json, &capacity);
  strncat(formatted_json, "}", 2);
  return formatted_json;
}

char *json_stringify(sql_result_t *result)
{
  if (result->num_rows == 1) {
    return serialize_row(result->column_info, result->rows[0], result->num_cols);
  }
}
