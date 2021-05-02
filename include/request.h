#ifndef REQUEST_H
#define REQUEST_H

typedef struct http_request {
  int socket_fd;
  enum { // TODO: finish adding these
    REQUEST_GET,
    REQUEST_POST,
  } method;
  char *url;
  // TODO: parse and store the headers
} http_request;

http_request *create_request();
int parse_request(int socket_fd, http_request *request);
void destroy_request(http_request *request);

#endif
