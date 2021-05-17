#ifndef REQUEST_H
#define REQUEST_H

#include "header.h"

typedef enum {
  REQUEST_GET,
  REQUEST_HEAD,
  REQUEST_POST,
  REQUEST_PUT,
  REQUEST_DELETE,
  REQUEST_PATCH,
  // REQUEST_CONNECT,
  // REQUEST_OPTIONS,
  // REQUEST_TRACE,
  REQUEST_METHODS // Total number
} request_method;

typedef struct {
  char *key;
  char *value;
} request_qfield;

typedef struct http_request {
  FILE *socket_file;
  request_method method;
  char *urlfull;
  char *url;
  http_headers headers;
  request_qfield *qfields;
  size_t qfields_size;
  char *body;
  size_t body_size;
} http_request;

http_request *create_request();
int parse_request(FILE *socket_file, http_request *request);
char *get_request_qfield(const http_request *request, char *key);
void destroy_request(http_request *request);

const char *get_method_name(request_method method);

#endif
