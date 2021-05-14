#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

#include "response.h"
#include "route.h"

// COLORS :D
#define NORMAL   "\x1B[39m"
#define RED      "\x1B[31m"
#define GREEN    "\x1B[32m"
#define YELLOW   "\x1B[33m"
#define BLUE     "\x1B[34m"
#define PORPLE   "\x1B[35m"
#define CYAN     "\x1B[36m"

void clear_response(http_response *response);

http_response *create_response()
{
  http_response *response = (http_response *)malloc(sizeof(http_response));
  response->body = NULL;
  response->headers.headers = NULL;
  response->status_code = OK;
  return response;
}

int send_response(
    http_response *response, const http_request *request,
    route_table *table, database_t *database)
{
  clear_response(response);

  response->socket_fd = request->socket_fd;
  FILE *socket_file = fdopen(dup(response->socket_fd), "wb");

  response->content_type = get_content_type(request->url);
  response->headers.headers = malloc(sizeof(http_header));
  response->headers.size = 0;
  response->headers.capacity = 1;

  route_target target = route_url(table, request->url);
  switch (target.type) {
    case ROUTE_TARGET_NONE:
      response->body = "<h1>404 error</h1>";
      response->body_size = strlen(response->body);
      response->status_code = NOT_FOUND;
      break;
    case ROUTE_TARGET_HANDLER:
      target.handler(request, response, database);
      break;
    case ROUTE_TARGET_FILE:
      serve_static(target.file, response);
      fclose(target.file);
      break;
  }

  const char *status_message = get_status_message(response->status_code);
  fprintf(socket_file, "HTTP/1.1 %s\r\n", status_message);
  fprintf(socket_file, "Content-Length: %zu\r\n", response->body_size);
  fprintf(socket_file, "Content-Type: %s\r\n", response->content_type);
  for (size_t i = 0; i < response->headers.size; i++) {
    fprintf(socket_file, "%s: %s\r\n",
        response->headers.headers[i].key,
        response->headers.headers[i].value);
  }
  fprintf(socket_file, "\r\n");
  fwrite(response->body, response->body_size, 1, socket_file);
  log_response(request, response);
  fclose(socket_file);

  return 0;
}

void clear_response(http_response *response)
{
  if (response->headers.headers) {
    free(response->headers.headers);
    response->headers.headers = NULL;
  }
  if (response->status_code == OK && response->body) {
    free(response->body);
    response->body = NULL;
  }
}

void destroy_response(http_response *response)
{
  clear_response(response);
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
  return "";
}

void serve_static(FILE *file, http_response *response)
{
  fseek(file, 0, SEEK_END);
  long fsize = ftell(file);
  rewind(file);
  response->body_size = fsize;

  response->body = (char *)malloc(fsize + 1);
  fread(response->body, 1, fsize, file);
  response->body[fsize] = 0;
  response->status_code = OK;
}

void log_response(const http_request *request, const http_response *response)
{
  char buffer[20];
  time_t now = time(NULL);
  struct tm *t = localtime(&now);
  strftime(buffer, sizeof(buffer), "%F %T", t);
  printf("[%s] " CYAN "\"%s %s HTTP/1.1\"%s %d " YELLOW "%ld\n" NORMAL,
    buffer,
    get_method_name(request->method),
    request->urlfull,
    response->status_code == OK ? GREEN : RED,
    response->status_code,
    response->body_size
  );
  fflush(stdout);
}

void log_error(const char *msg)
{
  char buffer[20];
  time_t now = time(NULL);
  struct tm *t = localtime(&now);
  strftime(buffer, sizeof(buffer), "%F %T", t);
  printf("[%s]" RED "ERROR: " NORMAL "%s\n", buffer, msg);
}
