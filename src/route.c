#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "route.h"

route_table *create_route_table(size_t initial_size)
{
  route_table *table = malloc(sizeof(route_table));

  table->size = 0;
  table->max_size = initial_size || 0x1;
  table->routes = (route_entry *)malloc(sizeof(route_entry) * initial_size);

  /* could move this stuff out of here, idk whatever you want */
  add_route(table, "/", "wwwroot/index.html");
  add_route(table, "/index.html", "wwwroot/index.html");
  add_route(table, "/test.js", "wwwroot/test.js");
  add_route(table, "/routed", "wwwroot/index.html");

  return table;
}

void add_route(route_table *table, char *url, char *file_name)
{
  if (table->size == table->max_size) {
    table->max_size *= 2;
    table->routes = realloc(table->routes, sizeof(route_entry) * table->max_size);
  }
  table->routes[table->size][0] = url;
  table->routes[table->size++][1] = file_name;
}

FILE *route_url(route_table *table, const char *url)
{
  for (unsigned i = 0; i < table->size; i++) {
    if (!strcmp(table->routes[i][0], url)) {
      FILE *file = fopen(table->routes[i][1], "rb");
      return file;
    }
  }
  return NULL;
}

void destroy_route_table(route_table *table)
{
  if (table->routes) {
    free(table->routes);
  }
  free(table);
}
