typedef struct {
  unsigned port;
  unsigned max_connections;
} http_server;

http_server *create_server(unsigned port, unsigned connections);
int run(http_server *server);