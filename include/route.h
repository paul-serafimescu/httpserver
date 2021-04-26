#ifndef ROUTE_H
#define ROUTE_H

#include <stdio.h>
#include <stdlib.h>

typedef enum {
  ROUTE_TYPE_FILE,
  ROUTE_TYPE_DIR
} route_type;

typedef struct {
  route_type type;
  char *url;
  char *path;
  size_t urllen;
  size_t pathlen;
} route_entry;

typedef struct {
  route_entry *routes;
  size_t size;
  size_t capacity;
} route_table;

route_table *create_route_table(size_t initial_size);
void add_file_route(route_table *table, char *url, char *file_name);
void add_dir_route(route_table *table, char *url, char *dir_name);
FILE *route_url(route_table *table, const char *url);
void destroy_route_table(route_table *table);

#endif
