#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

#include "response.h"
#include "route.h"

static const char *method_to_str(request_method method);

http_response *create_response()
{
  http_response *response = malloc(sizeof(http_response));
  response->body = NULL;
  response->status_code = OK;
  return response;
}

int send_response(http_response *response, const http_request *request, route_table *table)
{
  response->socket_fd = request->socket_fd;
  response->content_type = get_content_type(request->url);
  route_target target = route_url(table, request->url);
  switch (target.type) {
    case ROUTE_TARGET_NONE:
      response->body = "<h1>404 error</h1>";
      response->body_size = strlen(response->body);
      response->status_code = NOT_FOUND;
      break;
    case ROUTE_TARGET_HANDLER:
      target.handler(request, response);
      break;
    case ROUTE_TARGET_FILE:
      serve_static(target.file, response);
      fclose(target.file);
      break;
  }
  const char *status_message = get_status_message(response->status_code);
  char *response_text;
  int response_length = asprintf(&response_text, HTTP_FORMAT,
    status_message,
    response->content_type,
    response->body_size, response->body);
  write(response->socket_fd, response_text, response_length);
  log_response(request, response);
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

char *get_content_type(const char *url)
{
  url = strrchr(url, '.');
  if (url != NULL) {
    if (strcmp(url, ".js") == 0)
      return "application/javascript";
    if (strcmp(url, ".css") == 0)
      return "text/css";
    if (strcmp(url, ".json") == 0)
      return "application/json";
  }
  return "text/html";
}

void serve_static(FILE *file, http_response *response)
{
  fseek(file, 0, SEEK_END);
  long fsize = ftell(file);
  rewind(file);
  response->body_size = fsize;

  response->body = malloc(fsize + 1);
  fread(response->body, 1, fsize, file);
  response->body[fsize] = 0;
  response->status_code = OK;
}

void log_response(const http_request *request, const http_response *response)
{
  char buffer[100];
  time_t now = time(NULL);
  struct tm *t = localtime(&now);
  strftime(buffer, sizeof(buffer) - 1, "%Y-%m-%d %H:%M:%S", t);
  printf("[%s] \"%s %s HTTP/1.1\" %d %ld\n",
    buffer,
    method_to_str(request->method),
    request->url,
    response->status_code,
    response->body_size
  );
  fflush(stdout);
}

static const char *method_to_str(request_method method)
{
  switch (method) {
    case REQUEST_GET:
      return "GET";
    case REQUEST_POST:
      return "POST";
    default:
      return "UNKNOWN";
  }
}
