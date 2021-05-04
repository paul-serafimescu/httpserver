#ifndef REQUEST_H
#define REQUEST_H

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
} request_method;

typedef struct {
  char *key;
  char *value;
} http_header;

typedef struct http_request {
  int socket_fd;
  request_method method;
  char *urlfull;
  char *url;
  char *query_fields;
  http_header *headers;
  size_t headers_size;
} http_request;

http_request *create_request();
int parse_request(int socket_fd, http_request *request);
char *get_request_header(const http_request *request, char *key);
void destroy_request(http_request *request);

#endif
