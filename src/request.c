#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "request.h"

void clear_request(http_request *request);

http_request *create_request()
{
  http_request *request = malloc(sizeof(http_request));
  request->url = NULL;
  request->urlfull = NULL;
  return request;
}

int parse_request(int socket_fd, http_request *request)
{
  clear_request(request);

  request->socket_fd = socket_fd;

  FILE *socket_file = fdopen(dup(socket_fd), "rb");
  if (socket_file == NULL) {
    perror("fdopen");
    return -1;
  }
  char method_str[10];
  int scanned = fscanf(socket_file, "%9s %ms HTTP/1.1\r\n", method_str, &request->urlfull);
  if (scanned != 2) {
    fclose(socket_file);
    return -1;
  }

  if (!strcmp(method_str, "GET")) {
    request->method = REQUEST_GET;
  } else if (!strcmp(method_str, "HEAD")) {
    request->method = REQUEST_HEAD;
  } else if (!strcmp(method_str, "POST")) {
    request->method = REQUEST_POST;
  } else if (!strcmp(method_str, "PUT")) {
    request->method = REQUEST_PUT;
  } else if (!strcmp(method_str, "DELETE")) {
    request->method = REQUEST_DELETE;
  } else if (!strcmp(method_str, "PATCH")) {
    request->method = REQUEST_PATCH;
  } else {
    fclose(socket_file);
    return -1;
  }

  request->url = strdup(request->urlfull);
  char *querysep = strchr(request->url, '?');
  if (querysep) {
    *querysep = '\0';
    request->query_fields = querysep + 1;
  } else {
    request->query_fields = NULL;
  }

  fclose(socket_file);
  return 0;
}

void clear_request(http_request *request)
{
  if (request->urlfull) {
    free(request->urlfull);
    request->urlfull = NULL;
  }
  if (request->url) {
    free(request->url);
    request->url = NULL;
  }
}

void destroy_request(http_request *request)
{
  clear_request(request);
  free(request);
}
