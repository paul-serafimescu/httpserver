#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include "response.h"

void create_response(int socket_fd, unsigned status) {
  switch(status) {
    case OK:
      write(socket_fd, OK_HEADER, strlen(OK_HEADER));
      break;
    case BAD_REQUEST:
      write(socket_fd, BR_HEADER, strlen(BR_HEADER));
      break;
    default:
      write(socket_fd, NF_HEADER, strlen(NF_HEADER));
      break;
  }
}

void put_body(int socket_fd, char *path, unsigned status_code) {
  char *message = status_code == OK ?
    "hello, world!" :
      status_code == BAD_REQUEST ?
        "<h1>bad request</h1>" :
          "<h1>not found</h1>"; // yeah i'm proud of myself
  write(socket_fd, message, strlen(message));
  close(socket_fd);
}
