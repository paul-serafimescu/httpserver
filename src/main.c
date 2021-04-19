#include "server.h"

#define PORT 8000
#define MAX_CONNECTIONS 10

int main(int argc, char **argv) {

  http_server *server = create_server(PORT, MAX_CONNECTIONS);
  run(server);

  return 0;
}
