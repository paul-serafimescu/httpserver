#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "response.h"
#include "route.h"

http_response *create_response()
{
  http_response *response = malloc(sizeof(http_response));
  response->body = NULL;
  return response;
}

int send_response(http_response *response, const http_request *request, route_table *table)
{
  response->socket_fd = request->socket_fd;
  long fsize;
  FILE *file = serve(strcmp(request->url, "/") == 0 ? "/index.html" : request->url, response, table);
  if (file == NULL) {
    response->body = "<h1>404 error</h1>";
    response->body_size = strlen(response->body);
    response->status_code = NOT_FOUND;
    response->content_type = "text/html";
  } else {
    fsize = response->body_size;
    response->body = malloc(fsize + 1);
    fread(response->body, 1, fsize, file);
    fclose(file);

    response->body[fsize] = 0;
    response->status_code = OK;
    response->content_type = "text/html";
  }
  const char *status_message = get_status_message(response->status_code);
  char *response_text;
  int response_length = asprintf(&response_text, HTTP_FORMAT,
    status_message,
    response->content_type,
    response->body_size, response->body);
  print_response(response);
  write(response->socket_fd, response_text, response_length);
  close(response->socket_fd);
  free(response_text);

  return 0;
}

void destroy_response(http_response *response)
{
  if (response->status_code == OK && response->body) {
    free(response->body);
  }
  free(response);
}

/* helpers */

void print_response(http_response *response)
{
  printf("HTTP/1.1 %d\nContent-Type: %s\nContent-Length: %ld\n", response->status_code, response->content_type, response->body_size);
}

const char *get_status_message(int status_code)
{
  switch (status_code) {
    case OK:
      return "200 OK";
    case BAD_REQUEST:
      return "400 Bad Request";
    default:
      return "404 Not Found";
  }
}

FILE *serve(const char *url, http_response *response, route_table *table)
{
  long fsize;
  FILE *file = route_url(table, url);
  if (file == NULL) {
    return NULL;
  }
  fseek(file, 0, SEEK_END);
  fsize = ftell(file);
  rewind(file);

  response->body_size = fsize;

  return file;
}
