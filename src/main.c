#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <stdbool.h>

#define PORT 8000
#define CONN_REQUESTS 10

int main(int argc, char **argv) {
  int server_fd, new_socket, option_value = 1;
  long request;
  struct sockaddr_in request_address;
  int addrlen = sizeof(request_address);

  /* should probably modularize the response stuff */
  char *message = "HTTP/1.1 200 OK\nContent-Type: text/plain\nContent-Length: 12\n\nHello world!";

  /*
   * AF_INET is what can communicate, in this case IPv4
   * SOCK_STREAM is TCP
   * Internet Protocol is 0
   */

  if((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
    perror("socket");
    return 1;
  }

  if(setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &option_value, sizeof(option_value)) < 0) {
    perror("setting socket\n");
    return 1;
  }

  /**
   * set up socket
   * sin_family should be AF_INET for TCP
   * INADDR_ANY specifies any incoming message
   * htons(PORT) is the port to listen on
   */
  request_address.sin_family = AF_INET;
  request_address.sin_addr.s_addr = INADDR_ANY;
  request_address.sin_port = htons(PORT);

  memset(request_address.sin_zero, '\0', sizeof(request_address.sin_zero));

  if(bind(server_fd, (struct sockaddr*)&request_address, sizeof(request_address)) < 0) {
    perror("binding");
    return 1;
  }

  if(listen(server_fd, CONN_REQUESTS) < 0) {
    perror("listening");
    return 1;
  }

  printf("ready\n");

  while(true) {

    if((new_socket = accept(server_fd, (struct sockaddr*)&request_address, (socklen_t*)&addrlen)) < 0) {
      perror("in accept");
      return 1;
    }

    char buffer[30000] = {0};
    request = read(new_socket, buffer, 30000);
    write(new_socket, message, strlen(message));
    printf("sent\n"); // for some reason this is printing twice per request
    close(new_socket);
  }

  return 0;
}