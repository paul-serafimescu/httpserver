#ifndef SERVER_H
#define SERVER_H

#include <pthread.h>
#include "queue.h"
#include "route.h"
#include "database.h"

extern pthread_mutex_t queue_mutex;
extern pthread_cond_t client_exists;
extern queue_t *request_queue;

typedef struct {
  unsigned port;
  unsigned max_connections;
  route_table *table;
  database_t *database;
} http_server;

http_server *create_server(
    unsigned port, unsigned connections,
    route_table *table, database_t *database);
int init_worker_thread(pthread_t threads[], int num_threads, http_server *server);
int fini_worker_thread(pthread_t threads[], int num_threads);
int run(http_server *server);
void *handle_request(void *worker_id);
void destroy_server(http_server *server);

#endif
