#ifndef SERVER_H
#define SERVER_H

#include <pthread.h>
#include "queue.h"

pthread_mutex_t queue_mutex;
pthread_cond_t client_exists;
queue_t *request_queue;

typedef struct {
  unsigned port;
  unsigned max_connections;
} http_server;

http_server *create_server(unsigned port, unsigned connections);
int init_worker_thread(pthread_t threads[], int thread_ids[], int num_threads);
int run(http_server *server);
void *handle_request(void *worker_id);
void destroy_server(http_server *server);

#endif
