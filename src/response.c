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
  char *body, *file_contents, *target = strcmp(request->url, "/") == 0 ? "/index.html" : request->url;
  long fsize;
  FILE *file = serve(target);
  fseek(file, 0, SEEK_END);
  fsize = ftell(file);
  rewind(file);
  file_contents = malloc(fsize + 1);
  fread(file_contents, 1, fsize, file);
  fclose(file);
  file_contents[fsize] = 0;

  response->body = file_contents;
  response->body_size = fsize;
  response->status_code = OK;
  response->content_type = "text/html";

  return 0;
}

int send_response(int socket_fd, http_response *response)
{
  char buffer[30000] = {0};
  char *status_message = response->status_code == OK ?
    "OK" :
      response->status_code == BAD_REQUEST ?
        "Bad Request" :
          "Not Found";
  sprintf(buffer, HTTP_FORMAT, response->status_code, status_message, response->content_type, response->body_size, response->body);
  write(socket_fd, buffer, 30000);
  close(socket_fd);
}

void destroy_response(http_response *response)
{
  if (response->body) {
    free(response->body);
  }
  if (response->content_type) {
    free(response->content_type);
  }
  free(response);
}

/* helpers */

void print_response(http_response *response)
{
  printf("| HTTP/1.1 %d | Content-Type: %s | %s |\n", response->status_code, response->content_type, response->body);
}

FILE *serve(const char *file_name)
{
  char buffer[100] = {0};
  sprintf(buffer, "%s%s%s", "src/", STATIC_ROOT, file_name);
  return fopen(buffer, "rb");
}
