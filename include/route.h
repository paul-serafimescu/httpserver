#ifndef ROUTE_H
#define ROUTE_H

#include <stdio.h>
#include <stdlib.h>
#include <json-c/json_object.h>
#include "database.h"

// I'm sorry
typedef struct http_request http_request;
typedef struct http_response http_response;
typedef void (*http_handler)(
    const http_request *request, http_response *response, database_t *database, json_t params);

typedef enum {
  ROUTE_TYPE_FILE,
  ROUTE_TYPE_DIR,
  ROUTE_TYPE_HANDLER,
} route_type;

typedef struct {
  route_type type;
  union {
    struct {
      char *url;
      size_t urllen;
      char *path;
      size_t pathlen;
    };
    struct {
      char **params;
      size_t num_params;
      http_handler handler;
    };
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
    struct {
      http_handler handler;
      json_t params;
    };
  };
} route_target;

/*
 * Creates a new route table with a given initial capacity.
 * This initial capacity specifies exactly how many routes can be added before
 * an internal reallocation occurs.
 */
route_table *create_route_table(size_t initial_size);

/*
 * Adds a file route to the given table.
 * This maps an exact URL to a file on the filesystem (specified by relative
 * path from wwwroot). If route_url is called with a URL equal to the URL
 * passed here, the file specified here will be returned.
 */
void add_file_route(route_table *table, char *url, char *file_name);

/*
 * Adds a directory route to the given table.
 * This maps a URL 'directory' to a folder on the filesystem (specified by
 * relative path from wwwroot). If route_url is called with a URL which begins
 * with the URL passed here, a corresponding file in the folder passed here is
 * returned, found by replacing the URL 'prefix' passed here with the path of
 * the folder specified.
 */
void add_dir_route(route_table *table, char *url, char *dir_name);

/*
 * Adds a handler route to the given table.
 * This maps an exact URL to a handler callback. If route_url is called with a
 * URL equal to the URL passed here, the callback specified here will be
 * returned.
 */
void add_handler_route(route_table *table, char *url, http_handler handler);

/*
 * Decides what to do with a URL.
 * If the URL matches a file route or handler route exactly, the file or
 * handler is returned. Else, if a directory route matches the URL, the best
 * match is chosen (i.e. the matching directory route with longest URL), which
 * determines the file returned.
 * If there are no matching routes, or if a non-existent file was requested, a
 * 'NONE' action is returned.
 */
route_target route_url(route_table *table, const char *url);

/*
 * Destroys the given route table.
 * The table passed in is considered invalid after this call returns.
 */
void destroy_route_table(route_table *table);

#endif
