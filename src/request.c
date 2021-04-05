#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "request.h"

http_request *create_request()
{
  http_request *request = malloc(sizeof(http_request));
  request->url = NULL;
  return request;
}

int parse_request(int socket_fd, http_request *request)
{
  if (request->url) {
    free(request->url);
  }

  FILE *socket_file = fdopen(dup(socket_fd), "rb");
  if (socket_file == NULL) {
    perror("fdopen");
    return -1;
  }
  char *method_str = NULL;
  int scanned = fscanf(socket_file, "%ms %ms HTTP/1.1\r\n", &method_str, &request->url);
  if (scanned != 2) {
    if (method_str) {
      free(method_str);
    }
    fclose(socket_file);
    return -1;
  }

  if (strcmp(method_str, "GET")) {
    request->method = REQUEST_GET;
  } else if (strcmp(method_str, "POST")) {
    request->method = REQUEST_POST;
  } else {
    free(method_str);
    fclose(socket_file);
    return -1;
  }

  fclose(socket_file);
  return 0;
}

void destroy_request(http_request *request)
{
  if (request->url) {
    free(request->url);
  }
  free(request);
}
