#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "route.h"

static void resize_table(route_table *table);

route_table *create_route_table(size_t initial_size)
{
  route_table *table = malloc(sizeof(route_table));

  table->size = 0;
  table->capacity = initial_size > 0 ? initial_size : 1;
  table->routes = malloc(sizeof(route_entry) * table->capacity);

  return table;
}

static void resize_table(route_table *table)
{
  if (table->size == table->capacity) {
    table->capacity *= 2;
    table->routes = realloc(table->routes,
        sizeof(route_entry) * table->capacity);
  }
}

void add_file_route(route_table *table, char *url, char *file_name)
{
  resize_table(table);

  table->routes[table->size].url = url;
  table->routes[table->size].path = file_name;
  table->routes[table->size].pathlen = strlen(file_name);
  table->routes[table->size].type = ROUTE_TYPE_FILE;

  table->size++;
}

void add_dir_route(route_table *table, char *url, char *dir_name)
{
  resize_table(table);

  table->routes[table->size].url = url;
  table->routes[table->size].urllen = strlen(url);
  table->routes[table->size].path = dir_name;
  table->routes[table->size].pathlen = strlen(dir_name);
  table->routes[table->size].type = ROUTE_TYPE_DIR;

  table->size++;
}

FILE *route_url(route_table *table, const char *url)
{
  size_t urllen = strlen(url);
  size_t longest_match;
  size_t longest_prefix = 0;
  for (unsigned i = 0; i < table->size; i++) {
    switch (table->routes[i].type) {
      case ROUTE_TYPE_FILE:
        if (!strcmp(table->routes[i].url, url)) {
          char path[table->routes[i].pathlen + 9];
          sprintf(path, "wwwroot/%s", table->routes[i].path);
          return fopen(path, "rb");
        }
        break;
      case ROUTE_TYPE_DIR:
        ;
        size_t prefixlen = table->routes[i].urllen;
        if (prefixlen > longest_prefix &&
            !strncmp(table->routes[i].url, url, prefixlen)) {
          longest_match = i;
          longest_prefix = prefixlen;
        }
        break;
    }
  }
  if (longest_prefix > 0) {
    route_entry *route = table->routes + longest_match;
    char path[urllen - route->urllen + route->pathlen + 10];
    sprintf(path, "wwwroot/%s/%s", route->path, url + route->urllen);
    return fopen(path, "rb");
  }
  return NULL;
}

void destroy_route_table(route_table *table)
{
  free(table->routes);
  free(table);
}
