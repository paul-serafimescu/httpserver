#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

#include "response.h"
#include "route.h"

// COLORS :D
#define NORMAL   "\x1B[0m"
#define RED      "\x1B[31m"
#define GREEN    "\x1B[32m"
#define YELLOW   "\x1B[33m"
#define BLUE     "\x1B[34m"
#define CYAN     "\x1B[36m"

static const char *get_method_name(request_method method);

http_response *create_response()
{
  http_response *response = malloc(sizeof(http_response));
  response->body = NULL;
  response->status_code = OK;
  return response;
}

int send_response(
    http_response *response, const http_request *request,
    route_table *table, database_t *database)
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
      if (response->status_code == OK && response->body) {
        free(response->body);
        response->body = NULL;
      }
      target.handler(request, response, database);
      break;
    case ROUTE_TARGET_FILE:
      serve_static(target.file, response);
      fclose(target.file);
      break;
  }
  const char *status_message = get_status_message(response->status_code);
  char *response_header;
  int response_header_length = asprintf(&response_header, HTTP_FORMAT,
    status_message,
    response->content_type,
    response->body_size);
  write(response->socket_fd, response_header, response_header_length);
  write(response->socket_fd, response->body, response->body_size);
  log_response(request, response);
  close(response->socket_fd);
  free(response_header);

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

static const char *get_method_name(request_method method)
{
  switch (method) {
    case REQUEST_GET:
      return "GET";
    case REQUEST_HEAD:
      return "HEAD";
    case REQUEST_POST:
      return "POST";
    case REQUEST_PUT:
      return "PUT";
    case REQUEST_DELETE:
      return "DELETE";
    case REQUEST_PATCH:
      return "PATCH";
    default:
      return "UNKNOWN";
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

  response->body = malloc(fsize + 1);
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
  strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", t);
  printf("[%s]" RED "ERROR: " NORMAL "%s\n", buffer, msg);
}
