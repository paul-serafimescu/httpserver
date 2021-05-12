#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include "header.h"

int strcmp_caseless(const char *a, const char *b)
{
  size_t c = 0;
  while (a[c] && tolower(a[c]) == tolower(b[c])) {
    c++;
  }
  return a[c] - b[c];
}

char *get_header(const http_headers *headers, char *key)
{
  for (size_t i = 0; i < headers->size; i++) {
    if (!strcmp_caseless(headers->headers[i].key, key)) {
      return headers->headers[i].value;
    }
  }
  return NULL;
}

void set_header(http_headers *headers, char *key, char *value)
{
  for (size_t i = 0; i < headers->size; i++) {
    if (!strcmp_caseless(headers->headers[i].key, key)) {
      if (value) {
        headers->headers[i].value = value;
      } else {
        headers->headers[i].key = headers->headers[headers->size-1].key;
        headers->headers[i].value = headers->headers[headers->size-1].value;
        headers->size--;
      }
      return;
    }
  }
  if (value) {
    if (headers->size == headers->capacity) {
      headers->capacity *= 2;
      headers->headers =
        realloc(headers->headers, sizeof(http_header) * headers->capacity);
    }
    headers->headers[headers->size].key = key;
    headers->headers[headers->size].value = value;
    headers->size++;
  }
}
