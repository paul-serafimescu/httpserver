#ifndef ROUTE_H
#define ROUTE_H

#include <stdio.h>
#include <stdlib.h>

// I'm sorry
typedef struct http_request http_request;
typedef struct http_response http_response;
typedef void (*http_handler)(http_request *request, http_response *response);

typedef enum {
  ROUTE_TYPE_FILE,
  ROUTE_TYPE_DIR,
  ROUTE_TYPE_HANDLER,
} route_type;

typedef struct {
  route_type type;
  char *url;
  size_t urllen;
  union {
    struct {
      char *path;
      size_t pathlen;
    };
    http_handler handler;
  };
} route_entry;

typedef struct {
  route_entry *routes;
  size_t size;
  size_t capacity;
} route_table;

typedef enum {
  ROUTE_TARGET_NONE,
  ROUTE_TARGET_FILE,
  ROUTE_TARGET_HANDLER,
} route_target_type;

typedef struct {
  route_target_type type;
  union {
    FILE *file;
    http_handler handler;
  };
} route_target;

route_table *create_route_table(size_t initial_size);
void add_file_route(route_table *table, char *url, char *file_name);
void add_dir_route(route_table *table, char *url, char *dir_name);
void add_handler_route(route_table *table, char *url, http_handler handler);
route_target route_url(route_table *table, const char *url);
void destroy_route_table(route_table *table);

#endif
