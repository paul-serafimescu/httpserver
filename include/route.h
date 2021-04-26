#ifndef ROUTE_H
#define ROUTE_H

#include <stdio.h>
#include <stdlib.h>

#define WWW_ROOT "wwwroot/"

typedef char *route_entry[2];

typedef struct {
  route_entry *routes;
  size_t size;
  size_t max_size;
} route_table;

route_table *create_route_table(size_t initial_size);
void add_route(route_table *table, char *url, char *file_name);
FILE *route_url(route_table *table, const char *url);
void destroy_route_table(route_table *table);

char *prefix(char *original, char *prefix);

#endif
