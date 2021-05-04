#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <sys/wait.h>

#include "response.h"
#include "database.h"
#include "route.h"

static const char *method_to_str(request_method method);

http_response *create_response()
{
  http_response *response = (http_response *)malloc(sizeof(http_response));
  response->body = NULL;
  response->status_code = OK;
  return response;
}

void render(const http_request *request, http_response *response, sql_result_t *context, const char *template_name)
{
  if (response->status_code == OK && response->body) {
    free(response->body);
  }
  if (context == NULL) {
    response->body_size = asprintf(&response->body, "<h1>no idea what to do in here</h1>");
  } else {
    int fd[2];
    char buffer[3000], command[3000] = "m4 ", target[300]; // there has to be a better way to do this
    if (pipe(fd) < 0) {
      log_error("pipe failed");
      response->body = "<h1>500 error</h1>";
      response->body_size = strlen(response->body);
      response->status_code = INTERNAL_SERVER_ERROR;
      destroy_result(context);
      return;
    }
    pid_t pid = fork();
    if (pid < 0) {
      log_error("fork failed");
      response->body = "<h1>500 error</h1>";
      response->body_size = strlen(response->body);
      response->status_code = INTERNAL_SERVER_ERROR;
      destroy_result(context);
      return;
    } else if (pid == 0) {
      close(fd[0]);
      dup2(fd[1], STDOUT_FILENO);
      close(fd[1]);
      size_t i;
      for (i = 0; i < context->num_cols; i++) {
        char dfn_macro[300];
        switch (context->columns[i].type) {
          case TEXT:
            sprintf(dfn_macro, "\'-D__%s__=%s\' ", context->columns[i].name, context->columns[i].entries[0].t);
            break;
          case INTEGER:
            sprintf(dfn_macro, "\'-D__%s__=%d\' ", context->columns[i].name, context->columns[i].entries[0].i);
            break;
          case REAL:
            sprintf(dfn_macro, "\'-D__%s__=%f\' ", context->columns[i].name, context->columns[i].entries[0].d);
            break;
          default:
            break;
        }
        strcat(command, dfn_macro);
      }
      sprintf(target, "wwwroot/%s", template_name);
      strcat(command, target);
      char *argv[] = { "/usr/bin/bash", "-c", command, NULL };
      execvp(argv[0], argv);
      exit(errno);
    } else {
      close(fd[1]);
      if ((response->body_size = read(fd[0], buffer, sizeof(buffer))) < 0) {
        fprintf(stderr, "no output recovered\n");
      }
      response->body = malloc(response->body_size);
      memcpy(response->body, buffer, response->body_size);
      waitpid(pid, NULL, 0);
    }
    destroy_result(context);
  }
  response->status_code = OK;
}

int send_response(http_response *response, const http_request *request, route_table *table, database_t *database)
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
    case NOT_FOUND:
      return "404 Not Found";
    default:
      return "500 Internal Server Error";
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
}

void log_error(char *msg)
{
  char buffer[100];
  time_t now = time(NULL);
  struct tm *t = localtime(&now);
  strftime(buffer, sizeof(buffer) - 1, "%Y-%m-%d %H:%M:%S", t);
  printf("[%s] ERROR: %s\n", buffer, msg);
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
