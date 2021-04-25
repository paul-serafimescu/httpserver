#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "route.h"

route_table *create_route_table(size_t initial_size)
{
  route_table *table = malloc(sizeof(route_table));

  table->size = 0;
  table->capacity = initial_size > 0 ? initial_size : 1;
  table->routes = malloc(sizeof(route_entry) * table->capacity);

  return table;
}

void add_file_route(route_table *table, char *url, char *file_name)
{
  if (table->size == table->capacity) {
    table->capacity *= 2;
    table->routes = realloc(table->routes,
        sizeof(route_entry) * table->capacity);
  }
  table->routes[table->size].url = url;
  table->routes[table->size].path = file_name;

  table->routes[table->size].depth = 0;
  printf("DEBUG: %d %s\n", table->routes[table->size].depth, url);

  table->size++;
}

void add_dir_route(route_table *table, char *url, char *dir_name)
{
  if (table->size == table->capacity) {
    table->capacity *= 2;
    table->routes = realloc(table->routes,
        sizeof(route_entry) * table->capacity);
  }
  table->routes[table->size].url = url;
  table->routes[table->size].path = dir_name;

  table->routes[table->size].depth = 0;
  char *s = url;
  while (*s) {
    if (*s == '/') {
      table->routes[table->size].depth++;
    }
    s++;
  }
  printf("DEBUG: %d %s\n", table->routes[table->size].depth, url);

  table->size++;
}

FILE *route_url(route_table *table, const char *url)
{
  size_t url_len = strlen(url);
  if (!strcmp("/", url)) {
    return fopen("wwwroot/index.html", "rb");
  }
  for (unsigned i = 0; i < table->size; i++) {
    size_t entryurl_len = strlen(table->routes[i].url);
    if (!table->routes[i].depth) {
      if (!strcmp(table->routes[i].url, url)) {
        FILE *file = fopen(table->routes[i].path, "rb");
        return file;
      }
    } else {
      return NULL;
    }
  }
  return NULL;
}

void destroy_route_table(route_table *table)
{
  free(table->routes);
  free(table);
}
