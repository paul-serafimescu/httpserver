#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "database.h"

static void resize(char **str, size_t *capacity)
{
  *capacity *= 2;
  *str = realloc(*str, *capacity);
}

static char *serialize_row(column_t *fields, row_t row, size_t field_count)
{
  size_t i, size = 1, capacity = 7 * field_count, add_mem;
  char *formatted_json = malloc(capacity);
  strncpy(formatted_json, "{", 2);
  for (i = 0; i < field_count; i++) {
    size_t field_name_len = strlen(fields[i].name);
    while (capacity <= size + 3 + field_name_len) {
      resize(&formatted_json, &capacity);
    }
    strncat(formatted_json, "\"", 2);
    strncat(formatted_json, fields[i].name, field_name_len);
    strncat(formatted_json, "\":", 3);
    size += 3 + field_name_len;
    char *digits;
    switch (fields[i].type) {
      case INTEGER:
        add_mem = asprintf(&digits, "%d", row[i].i);
        while (capacity <= size + add_mem + 1) {
          resize(&formatted_json, &capacity);
        }
        strncat(formatted_json, digits, add_mem + 1);
        break;
      case REAL:
        add_mem = asprintf(&digits, "%f", row[i].d);
        while (capacity <= size + add_mem + 1) {
          resize(&formatted_json, &capacity);
        }
        strncat(formatted_json, digits, add_mem + 1);
        break;
      case TEXT:
        add_mem = asprintf(&digits, "\"%s\"", row[i].t);
        while (capacity <= size + add_mem + 1) {
          resize(&formatted_json, &capacity);
        }
        strncat(formatted_json, digits, add_mem + 1);
        break;
      default:
        while (capacity <= size + 5) {
          resize(&formatted_json, &capacity);
        }
        strncat(formatted_json, "null", 5);
        break; // consider NULL and BLOB handled
    }
    size += add_mem;
    free(digits);
    if (i != field_count - 1) {
      strncat(formatted_json, ",", 3);
      size++;
    }
  }
  while (capacity <= size + 1) {
    resize(&formatted_json, &capacity);
  }
  strncat(formatted_json, "}", 2);
  return formatted_json;
}

char *json_stringify(sql_result_t *result)
{
  size_t size = 2, capacity = size * 2, i, length;
  char *stringified_json = malloc(capacity), *entry;
  strncpy(stringified_json, "[", 2);
  for (i = 0; i < result->num_rows; i++) {
    entry = serialize_row(result->column_info, result->rows[i], result->num_cols);
    length = strlen(entry);
    while (capacity <= size + length + 1) {
      resize(&stringified_json, &capacity);
    }
    strncat(stringified_json, entry, length);
    if (i != result->num_rows - 1)
      strncat(stringified_json, ",", 2);
    size += length + 1;
    free(entry);
  }
  strncat(stringified_json, "]", 2);
  return stringified_json;
}
