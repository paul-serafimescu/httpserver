#ifndef REQUEST_H
#define REQUEST_H

typedef enum {
  REQUEST_GET,
  REQUEST_POST
} request_method;

typedef struct http_request {
  int socket_fd;
  request_method method; // TODO: finish adding these
  char *url;
  // TODO: parse and store the headers
} http_request;

http_request *create_request();
int parse_request(int socket_fd, http_request *request);
void destroy_request(http_request *request);

#endif
