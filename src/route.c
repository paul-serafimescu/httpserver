#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>
#include <stdbool.h>
#include <json-c/json_object.h>

#include "response.h"
#include "route.h"

#define MAX_PARAMS 16

static void resize_table(route_table *table);
static char **tokenize_url(char *src, const char *delim, size_t *size);
static bool match(char **url_p, const size_t num_params, const char *url, char ***url_split, size_t *size);

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

static char **tokenize_url(char *src, const char *delim, size_t *size)
{
  char *token, **params = malloc(MAX_PARAMS * sizeof(char *)), *ptr;
  if (src[0] == '/') {
    ptr = src + 1;
  } else {
    ptr = src;
  }
  src = strdup(ptr);
  for (*size = 0; (token = strsep(&src, delim));) {
    if (*size == MAX_PARAMS) break;
    params[(*size)++] = token;
  }
  return params;
}

static bool match(char **url_p, const size_t num_params, const char *url, char ***url_split, size_t *size)
{
  (*url_split) = tokenize_url(url, "/", size);
  if (*size != num_params)
    return false;
  for (size_t i = 0; i < num_params; i++) {
    if (url_p[i][0] != '{') { // normal url component
      if (strcmp(url_p[i], (*url_split)[i]))
        return false; // not the same
    } else { // parametrized url component
      char type = strchr(url_p[i], ':')[1];
      switch (type) {
        case 'd':
          if (!atoi((*url_split)[i])) {
            return false;
          }
          break;
        case 's':
          break;
        default:
          return false;
      }
    }
  }
  return true;
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

void add_handler_route(route_table *table, char *url, http_handler handler)
{
  resize_table(table);

  size_t num_params;

  table->routes[table->size].type = ROUTE_TYPE_HANDLER;
  table->routes[table->size].handler = handler;
  table->routes[table->size].params = tokenize_url(url, "/", &num_params);
  table->routes[table->size].num_params = num_params;

  table->size++;
}

route_target route_url(route_table *table, const char *url)
{
  size_t urllen = strlen(url);
  size_t longest_match;
  size_t longest_prefix = 0;
  route_target target;
  json_t params = json_object_new_object();

  for (unsigned i = 0; i < table->size; i++) {
    switch (table->routes[i].type) {
      case ROUTE_TYPE_FILE:
        if (!strcmp(table->routes[i].url, url)) {
          char path[table->routes[i].pathlen + 9];
          sprintf(path, "wwwroot/%s", table->routes[i].path);
          target.file = fopen(path, "rb");
          target.type = target.file ? ROUTE_TARGET_FILE : ROUTE_TARGET_NONE;
          json_object_put(params);
          return target;
        }
        break;
      case ROUTE_TYPE_HANDLER:;
        char **split_url;
        size_t size;
        if (match(table->routes[i].params, table->routes[i].num_params, url, &split_url, &size)) {
          target.type = ROUTE_TARGET_HANDLER;
          target.handler = table->routes[i].handler;
          char **p = table->routes[i].params;
          size_t num_params = table->routes[i].num_params;
          for (size_t j = 0; j < num_params; j++) {
            if (p[j][0] != '{') continue;
            char *tmp = strchr(p[j], ':');
            if (tmp == NULL) {
              // handle this user error
            }
            char type = (tmp + 1)[0];
            json_t entry = NULL;
            switch (type) {
              case 'd':;
                long val = atol(split_url[j]);
                entry = json_object_new_int64(val);
                break;
              case 's':
                entry = json_object_new_string(split_url[j]);
                break;
              default:
                // no clue what else there is
                break;
            }
            size_t i, value_length = strlen(p[j]) + 1;
            char *value = malloc(value_length);
            for (i = 1; p[j][i] != ':'; i++) {
              value[i - 1] = p[j][i];
            }
            value[i - 1] = '\0';
            json_object_object_add(params, value, entry);
            free(value);
          }
          target.params = params;
          free(split_url[0]);
          free(split_url);
          return target;
        }
        free(split_url[0]);
        free(split_url);
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
    target.file = fopen(path, "rb");
    target.type = target.file ? ROUTE_TARGET_FILE : ROUTE_TARGET_NONE;
    json_object_put(params);
    return target;
  }
  json_object_put(params);
  target.type = ROUTE_TARGET_NONE;
  return target;
}

void destroy_route_table(route_table *table)
{
  for (size_t i = 0; i < table->size; i++) {
    if (table->routes[i].type == ROUTE_TYPE_HANDLER) {
      free(table->routes[i].params[0]);
      free(table->routes[i].params);
    }
  }
  free(table->routes);
  free(table);
}
