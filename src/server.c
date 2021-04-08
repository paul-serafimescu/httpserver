#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <stdbool.h>

#include "server.h"
#include "request.h"
#include "response.h"

http_server *create_server(unsigned port, unsigned connections)
{
  http_server *server = malloc(sizeof(http_server));
  server->port = port;
  server->max_connections = connections;
  return server;
}

int run(http_server *server)
{
  int server_fd, new_socket, option_value = 1;
  struct sockaddr_in request_address;
  size_t addrlen = sizeof(request_address);

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
    perror("setting socket");
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
  request_address.sin_port = htons(server->port);

  memset(request_address.sin_zero, '\0', sizeof(request_address.sin_zero));

  if(bind(server_fd, (struct sockaddr*)&request_address, addrlen) < 0) {
    perror("binding");
    return 1;
  }

  if(listen(server_fd, server->max_connections) < 0) {
    perror("listening");
    return 1;
  }

  printf("ready\n");

  http_request *request = create_request();

  while(true) {

    if((new_socket = accept(server_fd, (struct sockaddr*)&request_address, (socklen_t*)&addrlen)) < 0) {
      perror("in accept");
      return 1;
    }

    if (parse_request(new_socket, request) == 0) {
      http_response *response = create_response();
      add_body(response, request);
      send_response(response);
      destroy_response(response);
    }
  }
}

void destroy_server(http_server *server)
{
  free(server);
}
