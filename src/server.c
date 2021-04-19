#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <stdbool.h>
#include <pthread.h>

#include "server.h"
#include "request.h"
#include "response.h"
#include "route.h"
#include "utils.h"

#define NUM_THREADS 10

pthread_mutex_t queue_mutex;
pthread_cond_t client_exists;
queue_t *request_queue;

http_server *create_server(unsigned port, unsigned connections)
{
  http_server *server = malloc(sizeof(http_server));
  server->port = port;
  server->max_connections = connections;
  return server;
}

void *handle_request(void *worker_id)
{
  http_request *request = create_request();
  route_table *table = create_route_table(); // maybe this should be a global variable idk
  int socket;
  while (true) {
    socket = -1;
    pthread_mutex_lock(&queue_mutex);
    if (is_empty(request_queue))
      pthread_cond_wait(&client_exists, &queue_mutex);
    else
      dequeue(request_queue, &socket);
    pthread_mutex_unlock(&queue_mutex);
    // printf("socket #%d\n", socket);
    if (socket >= 0) {
      if (parse_request(socket, request) == 0) {
        printf("%s\n", request->url);
        http_response *response = create_response();
        send_response(response, request, table);
        destroy_response(response);
      }
    }
  }
  destroy_request(request);
}

int run(http_server *server)
{
  int server_fd, new_socket, flag = 0, num_threads = NUM_THREADS, option_value = 1;
  pthread_t worker_threads[NUM_THREADS];
  int thread_ids[NUM_THREADS];
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

  /* initialize request queues */
  request_queue = create_queue();
  pthread_mutex_init(&queue_mutex, NULL);
  pthread_cond_init(&client_exists, NULL);

  if (init_worker_thread(worker_threads, thread_ids, num_threads) < 0) {
    printf("help\n"); // idk
  }

  if(listen(server_fd, server->max_connections) < 0) {
    perror("listening");
    return 1;
  }

  printf("ready\n");

  while(true) {

    if((new_socket = accept(server_fd, (struct sockaddr*)&request_address, (socklen_t*)&addrlen)) < 0) {
      perror("in accept");
      return 1;
    }
    pthread_mutex_lock(&queue_mutex);
    if (is_empty(request_queue))
      flag = 1;
    enqueue(request_queue, new_socket);
    if (flag)
      pthread_cond_broadcast(&client_exists);
    flag = 0;
    pthread_mutex_unlock(&queue_mutex);
  }
}

int init_worker_thread(pthread_t threads[], int thread_ids[], int num_threads)
{
  int i;
  for (i = 0; i < num_threads; i++) {
    thread_ids[i] = i;
    if (pthread_create(&threads[i], NULL, handle_request, &thread_ids[i])) {
      // idk how to handle this tbh
      return -1;
    }
  }

  return 0;
}

void destroy_server(http_server *server)
{
  free(server);
}
