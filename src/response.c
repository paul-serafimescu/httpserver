#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "response.h"

http_response *create_response()
{
  http_response *response = malloc(sizeof(http_response));
  response->body = NULL;
  return response;
}

int add_body(http_response *response, const http_request *request)
{
  char *target = strcmp(request->url, "/") == 0 ? "/index.html" : request->url;
  long fsize;
  FILE *file = serve(target, response);
  if (file == NULL) {
    response->body = "<h1>404 error</h1>";
    response->body_size = strlen(response->body) + 1;
    response->status_code = NOT_FOUND;
    response->content_type = "text/html";

    return -1;
  }
  fsize = response->body_size;
  response->body = malloc(fsize + 1);
  fread(response->body, 1, fsize, file);
  fclose(file);

  response->body[fsize] = 0;
  response->status_code = OK;
  response->content_type = "text/html";

  return 0;
}

int send_response(int socket_fd, http_response *response)
{
  char *status_message = response->status_code == OK ?
    "OK" :
      response->status_code == BAD_REQUEST ?
        "Bad Request" :
          "Not Found";
  char *response_text;
  int response_length = asprintf(&response_text, HTTP_FORMAT,
    response->status_code, status_message,
    response->content_type,
    response->body_size, response->body);
  print_response(response);
  write(socket_fd, response_text, response_length);
  close(socket_fd);
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

FILE *serve(const char *file_name, http_response *response)
{
  long fsize;
  char *fs_name;
  asprintf(&fs_name, "%s%s", STATIC_ROOT, file_name);
  FILE *file = fopen(fs_name, "rb");
  free(fs_name);
  if (file == NULL) {
    return NULL;
  }
  fseek(file, 0, SEEK_END);
  fsize = ftell(file);
  rewind(file);

  response->body_size = fsize;

  return file;
}
