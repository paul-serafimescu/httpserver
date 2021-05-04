#define _GNU_SOURCE
#include "server.h"
#include "request.h"
#include "response.h"
#include "route.h"
#include "database.h"

#define PORT 8000
#define MAX_CONNECTIONS 10

void test_handler(const http_request *request, http_response *response, database_t *database);
// void a_handler(const http_request *request, http_response *response, database_t *database);

int main(int argc, char **argv) {

  database_t *database = create_cursor("db.sqlite3");
  route_table *routes = create_route_table(0);

  add_dir_route(routes, "/", ".");
  add_dir_route(routes, "/routed/", ".");
  add_file_route(routes, "/", "index.html");
  add_file_route(routes, "/routed", "index.html");
  add_file_route(routes, "/routed/", "index.html");
  add_handler_route(routes, "/handle", test_handler);
  // add_handler_route(routes, "/help", a_handler);

  http_server *server = create_server(PORT, MAX_CONNECTIONS, routes, database);
  run(server);

  return 0;
}

void test_handler(const http_request *request, http_response *response, database_t *database)
{
  sql_result_t *context = select_all(database, "Test3");
  return render(request, response, context, "lordhavemercyonme.m4");
}

/*void a_handler(const http_request *request, http_response *response, database_t *database)
{
  return render(request, response, NULL);
}*/
