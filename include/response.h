#ifndef RESPONSE_H
#define RESPONSE_H

#include "request.h"
#include "route.h"
#include "database.h"

#define HTTP_FORMAT "HTTP/1.1 %s\r\nContent-Type: %s\r\nContent-Length: %ld\r\n\r\n"

typedef struct http_response {
  int socket_fd;
  enum {
    OK = 200,
    NOT_FOUND = 404,
    BAD_REQUEST = 400
  } status_code;
  char *content_type;
  char *body;
  long body_size;
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
void serve_static(FILE *file, http_response *response);
void log_error(const char *msg);

#endif
