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
  REQUEST_METHODS // Total number
} request_method;

typedef struct {
  char *key;
  char *value;
} request_qfield;

typedef struct {
  char *key;
  char *value;
} request_header;

typedef struct http_request {
  int socket_fd;
  request_method method;
  char *urlfull;
  char *url;
  request_header *headers;
  size_t headers_size;
  request_qfield *qfields;
  size_t qfields_size;
  char *body;
  size_t body_size;
} http_request;

http_request *create_request();
int parse_request(int socket_fd, http_request *request);
char *get_request_header(const http_request *request, char *key);
char *get_request_qfield(const http_request *request, char *key);
void destroy_request(http_request *request);

const char *get_method_name(request_method method);

#endif
