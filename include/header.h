#ifndef HEADER_H
#define HEADER_H

typedef struct {
  char *key;
  char *value;
} http_header;

typedef struct {
  http_header *headers;
  size_t size;
  size_t capacity;
} http_headers;

/*
 * Compares two strings, ignoring case.
 * Returns 0 if the strings are equivalent, or some non-zero value otherwise.
 */
int strcmp_caseless(const char *a, const char *b);

/*
 * Gets a header's value.
 * Returns a reference to the header value if present, or NULL otherwise.
 */
char *get_header(const http_headers *headers, char *key);

/*
 * Sets a header's value.
 * If the new value is NULL, the header whose name is given is deleted.
 * If the header is being created, the string pointed to by `key` must remain
 * valid and unchanged while the header is in the http_header struct.
 * If non-NULL, the string pointed to by `value` must remain valid and
 * unchanged while the header is in the http_header struct.
 */
void set_header(http_headers *headers, char *key, char *value);

#endif
