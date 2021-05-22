#define _GNU_SOURCE
#include <json-c/json_object.h>
#include <json-c/json_tokener.h>

#include "server.h"
#include "request.h"
#include "response.h"
#include "route.h"
#include "database.h"

#define PORT 8000
#define MAX_CONNECTIONS 10

void example_handler(const http_request *request, http_response *response, database_t *database, json_t params);

int main(int argc, char **argv) {
  (void)argc;
  (void)argv;

  route_table *table = create_route_table(0);
  add_dir_route(table, "/", "example-frontend/build");
  add_file_route(table, "/", "example-frontend/build/index.html");
  add_handler_route(table, "/users", example_handler);

  database_t *database = create_cursor("db.sqlite3");

  http_server *server = create_server(PORT, MAX_CONNECTIONS, table, database);
  run(server);
  destroy_server(server);

  destroy_route_table(table);
  destroy_cursor(database);

  return 0;
}

void example_handler(const http_request *request, http_response *response, database_t *database, json_t params)
{
  switch (request->method) {
    case REQUEST_GET:;
      json_t users = select_all(database, "Users");
      response->body_size = asprintf(&response->body, "%s", json_object_to_json_string_ext(users, JSON_C_TO_STRING_PLAIN));
      response->status_code = OK;
      response->content_type = "application/json";
      json_object_put(users);
      break;
    case REQUEST_POST:
      if (request->body == NULL) {
        response->body_size = asprintf(&response->body, "<p>invalid request</p>");
        response->status_code = BAD_REQUEST;
        return;
      }
      json_t body = json_tokener_parse(request->body);
      int rv = insert_into_table(database, "Users", "sss",
        json_object_get_string(json_object_object_get(body, "name")), 0,
        json_object_get_string(json_object_object_get(body, "password")), 0,
        json_object_get_string(json_object_object_get(body, "email")), 0
      );
      if (rv > 0) {
        response->body_size = asprintf(&response->body, "{\"id\":%d}", rv);
        response->status_code = OK;
        return;
      }
      response->status_code = BAD_REQUEST;
      break;
  }
}
