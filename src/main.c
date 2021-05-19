#define _GNU_SOURCE
#include <json-c/json_object.h>

#include "server.h"
#include "request.h"
#include "response.h"
#include "route.h"
#include "database.h"

#define PORT 8000
#define MAX_CONNECTIONS 10

void test_handler(const http_request *request, http_response *response, database_t *database, json_t params);

int main(int argc, char **argv) {
  (void)argc;
  (void)argv;

  route_table *table = create_route_table(0);
  add_dir_route(table, "/", ".");
  add_dir_route(table, "/routed/", ".");
  add_file_route(table, "/", "index.html");
  add_file_route(table, "/routed", "index.html");
  add_file_route(table, "/routed/", "index.html");
  add_handler_route(table, "/handle/{testing:s}/{id:d}", test_handler);

  database_t *database = create_cursor("db.sqlite3");

  http_server *server = create_server(PORT, MAX_CONNECTIONS, table, database);
  run(server);
  destroy_server(server);

  destroy_route_table(table);
  destroy_cursor(database);

  return 0;
}

void test_handler(const http_request *request, http_response *response, database_t *database, json_t params)
{
  static int count = 0;
  printf("%s\n", json_object_to_json_string_ext(params, JSON_C_TO_STRING_PLAIN));
  char *name = get_request_qfield(request, "name");
  char *host = get_header(&request->headers, "host");
  int x = insert_into_table(database, "Test3", "sd", "mary", 0, 300 + count);
  update_by_id(database, "Test3", 1, "age = age + ?", "d", 300);
  json_t r = select_all(database, "Test3");
  // printf("%s\n", json_object_to_json_string_ext(r, JSON_C_TO_STRING_PLAIN));
  json_object_put(r);
  delete_by_id(database, "Test3", x);
  if (name) {
    set_header(&response->headers, "Set-Cookie", name);
  } else {
    name = get_header(&request->headers, "Cookie");
  }
  response->body_size =
    asprintf(&response->body, "<p>count=%d name=%s Host=%s</p>",
        count, name, host);
  response->status_code = OK;
  count++;
}
