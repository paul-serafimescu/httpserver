#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <unistd.h>

#include "request.h"

void clear_request(http_request *request);

http_request *create_request()
{
  http_request *request = malloc(sizeof(http_request));
  request->url = NULL;
  request->urlfull = NULL;
  request->qfields = NULL;
  request->headers = NULL;
  request->body = NULL;
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
  if (fscanf(socket_file, "%9s %ms HTTP/1.1\r\n", method_str, &request->urlfull) != 2) {
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
    request->qfields = malloc(sizeof(request_qfield));
    request->qfields_size = 0;
    size_t qfields_capacity = 1;
    char *s = querysep + 1;
    int url_ended = 0;
    while (!url_ended && *s) {
      char *key_end = strpbrk(s, "=");
      if (!key_end) break;
      *key_end = '\0';
      size_t value_length = strcspn(key_end + 1, ";&");
      if (!*(key_end + 1 + value_length)) {
        url_ended = 1;
      }
      *(key_end + 1 + value_length) = '\0';
      if (request->qfields_size == qfields_capacity) {
        qfields_capacity *= 2;
        request->qfields =
          realloc(request->qfields, sizeof(request_qfield) * qfields_capacity);
      }
      request->qfields[request->qfields_size].key = s;
      request->qfields[request->qfields_size].value = key_end + 1;
      request->qfields_size++;
      s = key_end + 1 + value_length + 1;
    }
  } else {
    request->qfields = NULL;
    request->qfields_size = 0;
  }

  char *key;
  char *value;
  request->headers = malloc(sizeof(http_header));
  request->headers_size = 0;
  size_t headers_capacity = 1;
  while (fscanf(socket_file, "%m[^\r:]: %m[^\r]", &key, &value) == 2) {
    fgetc(socket_file);
    fgetc(socket_file);
    if (request->headers_size == headers_capacity) {
      headers_capacity *= 2;
      request->headers =
        realloc(request->headers, sizeof(http_header) * headers_capacity);
    }
    request->headers[request->headers_size].key = key;
    request->headers[request->headers_size].value = value;
    request->headers_size++;
  }
  fgetc(socket_file);
  fgetc(socket_file);

  if (request->method == REQUEST_POST ||
      request->method == REQUEST_PUT ||
      request->method == REQUEST_DELETE ||
      request->method == REQUEST_PATCH) {
    // No i don't know what a transfer encoding is
    char *content_length = get_request_header(request, "content-length");
    if (content_length) {
      request->body_size = atoi(content_length);
      request->body = malloc(request->body_size);
      fread(request->body, 1, request->body_size, socket_file);
    }
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
  if (request->qfields) {
    free(request->qfields);
    request->qfields = NULL;
  }
  if (request->headers) {
    for (size_t i = 0; i < request->headers_size; i++) {
      free(request->headers[i].key);
      free(request->headers[i].value);
    }
    free(request->headers);
    request->headers = NULL;
  }
  if (request->body) {
    free(request->body);
    request->body = NULL;
  }
}

void destroy_request(http_request *request)
{
  clear_request(request);
  free(request);
}

char *get_request_header(const http_request *request, char *key)
{
  for (size_t i = 0; i < request->headers_size; i++) {
    size_t c = 0;
    while (request->headers[i].key[c] &&
           tolower(request->headers[i].key[c]) == tolower(key[c])) {
      c++;
    }
    if (!request->headers[i].key[c] && !key[c]) {
      return request->headers[i].value;
    }
  }
  return NULL;
}

char *get_request_qfield(const http_request *request, char *key)
{
  for (size_t i = 0; i < request->qfields_size; i++) {
    if (!strcmp(request->qfields[i].key, key)) {
      return request->qfields[i].value;
    }
  }
  return NULL;
}
