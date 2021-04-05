#ifndef RESPONSE_H
#define RESPONSE_H

#include "request.h"

#define HTTP_FORMAT "HTTP/1.1 %d %s\r\nContent-Type: %s\r\nContent-Length: %ld\r\n\r\n%s"
#define STATIC_ROOT "wwwroot"

typedef struct {
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
int add_body(http_response *response, const http_request *request);
int send_response(int socket_fd, http_response *response);
void destroy_response(http_response *response);

/* helpers */
void print_response(http_response *response);
FILE *serve(const char *file_name);

#endif