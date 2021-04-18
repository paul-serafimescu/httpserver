#ifndef ROUTE_H
#define ROUTE_H

#include <stdio.h>

#define STATIC_ROOT "wwwroot"

typedef char *route_entry[2];

typedef struct {
	route_entry* routes;
	int routes_size;
} route_table;

route_table *create_route_table(); // TODO: make this accept input
FILE *route_url(route_table *table, const char *url);
void destroy_route_table(route_table *table);

#endif
