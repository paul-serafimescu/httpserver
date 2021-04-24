#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "route.h"

route_table *create_route_table(size_t initial_size)
{
  route_table *table = malloc(sizeof(route_table));

  table->size = 0;
  table->max_size = initial_size > 0 ? initial_size : 1;
  table->routes = (route_entry *)malloc(sizeof(route_entry) * initial_size);

  // could move this stuff out of here, idk whatever you want
  // we should also handle "static" files like js and css separately
  // so we don't have to explicitly declare routes for those
  add_route(table, "/", "index.html");
  add_route(table, "/index.html", "index.html");
  add_route(table, "/test.js", "test.js");
  add_route(table, "/test.css", "test.css");
  add_route(table, "/routed", "index.html");

  return table;
}

char *prefix(char *original, char *prefix)
{
  size_t prefix_length = strlen(prefix);
  size_t original_length = strlen(original);
  char *new_string = (char *)malloc(prefix_length + original_length + 1);
  strcpy(new_string, prefix);
  strcat(new_string, original);
  return new_string;
}

void add_route(route_table *table, char *url, char *file_name)
{
  char *full_name = prefix(file_name, WWW_ROOT);
  if (table->size + 1 == table->max_size) {
    table->max_size *= 2;
    table->routes = realloc(table->routes, sizeof(route_entry) * table->max_size);
  }
  table->routes[table->size][0] = url;
  table->routes[table->size++][1] = full_name;
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
