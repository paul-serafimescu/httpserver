#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "route.h"

route_table *create_route_table()
{
  route_table *table = malloc(sizeof(route_table));

  table->routes_size = 4;
  table->routes = malloc(sizeof(route_entry) * table->routes_size);

  table->routes[0][0] = "/";
  table->routes[0][1] = "wwwroot/index.html";
  table->routes[1][0] = "/index.html";
  table->routes[1][1] = "wwwroot/index.html";
  table->routes[2][0] = "/test.js";
  table->routes[2][1] = "wwwroot/test.js";
  table->routes[3][0] = "/routed";
  table->routes[3][1] = "wwwroot/index.html";

  return table;
}

FILE *route_url(route_table *table, const char *url)
{
  for (int i = 0; i < table->routes_size; i++) {
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
