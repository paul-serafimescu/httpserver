#ifndef RESPONSE_H
#define RESPONSE_H

#include "request.h"
#include "route.h"
#include "database.h"
#include "header.h"

typedef struct {
  char *key;
  char *value;
} response_header;

typedef struct http_response {
  FILE *socket_file;
  enum {
    OK = 200,
    NOT_FOUND = 404,
    BAD_REQUEST = 400
  } status_code;
  http_headers headers;
  char *body;
  size_t body_size;
  char *content_type;
} http_response;

http_response *create_response();
int send_response(
    http_response *response, const http_request *request,
    route_table *table, database_t *database);
void destroy_response(http_response *response);

/* helpers */
void log_response(const http_request *request, const http_response *response);
const char *get_status_message(int status_code);
char *get_content_type(const char *url);
char *get_response_header(const http_response *response, char *key);
void set_response_header(const http_response *response, char *key, char *value);
void serve_static(FILE *file, http_response *response);
void log_error(const char *msg);

#endif
