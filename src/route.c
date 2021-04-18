#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>

#include "route.h"

route_table *create_route_table()
{
  route_table *table = malloc(sizeof(route_table));
  table->routes = NULL;
  return table;
}

FILE *route_url(route_table *table, const char *url)
{
  char *fs_name;
  asprintf(&fs_name, "%s%s", STATIC_ROOT, url);
  FILE *file = fopen(fs_name, "rb");
  free(fs_name);
  return file;
}

void destroy_route_table(route_table *table)
{
  if (table->routes) {
    free(table->routes);
  }
  free(table);
}
