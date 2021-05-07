#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <stdbool.h>
#include <pthread.h>
#include <signal.h>

#include "server.h"
#include "request.h"
#include "response.h"
#include "route.h"
#include "utils.h"
#include "database.h"

#define NUM_THREADS 10

pthread_mutex_t queue_mutex;
pthread_cond_t client_exists;
queue_t *request_queue;

void test_handler(const http_request *request, http_response *response);

http_server *create_server(
    unsigned port, unsigned connections,
    route_table *table, database_t *database)
{
  http_server *server = malloc(sizeof(http_server));
  server->port = port;
  server->max_connections = connections;
  server->table = table;
  server->database = database;
  return server;
}

void *handle_request(void *input)
{
  http_server *server = (http_server *)input;
  http_request *request = create_request();
  int socket;
  int running = 1;
  while (running) {
    pthread_mutex_lock(&queue_mutex);
    while (is_empty(request_queue)) {
      pthread_cond_wait(&client_exists, &queue_mutex);
    }
    dequeue(request_queue, &socket);
    if (socket < 0) {
      enqueue(request_queue, socket);
      pthread_mutex_unlock(&queue_mutex);
      running = 0;
    } else {
      pthread_mutex_unlock(&queue_mutex);
      if (parse_request(socket, request) == 0) {
        http_response *response = create_response();
        send_response(response, request, server->table, server->database);
        destroy_response(response);
      }
    }
  }
  destroy_request(request);
  return NULL;
}

int run(http_server *server)
{
  int server_fd, new_socket, num_threads = NUM_THREADS, option_value = 1;
  pthread_t worker_threads[NUM_THREADS];
  struct sockaddr_in request_address;
  size_t addrlen = sizeof(request_address);

  void sig_handle(int signal) { (void)signal; }
  struct sigaction act;
  act.sa_handler = sig_handle;
  act.sa_flags = 0;
  sigemptyset(&act.sa_mask);
  // sigaction(SIGINT, &act, NULL);

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

  if (init_worker_thread(worker_threads, num_threads, server) < 0) {
    printf("help\n"); // idk
  }

  if (listen(server_fd, server->max_connections) < 0) {
    perror("listening");
    return 1;
  }

  printf("ready\n");

  while(true) {

    if((new_socket = accept(server_fd, (struct sockaddr*)&request_address, (socklen_t*)&addrlen)) < 0) {
      perror("in accept");
      break;
    }

    pthread_mutex_lock(&queue_mutex);
    enqueue(request_queue, new_socket);
    pthread_cond_signal(&client_exists);
    pthread_mutex_unlock(&queue_mutex);

  }

  pthread_mutex_lock(&queue_mutex);
  enqueue(request_queue, -1);
  pthread_cond_broadcast(&client_exists);
  pthread_mutex_unlock(&queue_mutex);

  fini_worker_thread(worker_threads, num_threads);

  pthread_mutex_destroy(&queue_mutex);
  pthread_cond_destroy(&client_exists);
  destroy_queue(request_queue);
  return 0;
}

int init_worker_thread(pthread_t threads[], int num_threads, http_server *server)
{
  int i;
  for (i = 0; i < num_threads; i++) {
    if (pthread_create(&threads[i], NULL, handle_request, server)) {
      // idk how to handle this tbh
      return -1;
    }
  }

  return 0;
}

int fini_worker_thread(pthread_t threads[], int num_threads)
{
  int i;
  for (i = 0; i < num_threads; i++) {
    if (pthread_join(threads[i], NULL)) {
      return -1;
    }
  }

  return 0;
}

void destroy_server(http_server *server)
{
  free(server);
}
