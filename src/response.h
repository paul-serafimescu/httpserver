#define OK_HEADER "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: 12\r\n\r\n"
#define NF_HEADER "HTTP/1.1 404 Not Found\r\nContent-Type: text/html\r\nContent-Length: 18\r\n\r\n"
#define BR_HEADER "HTTP/1.1 400 Bad Request\r\nContent-Type: text/html\r\nContent-Length: 20\r\n\r\n"

enum status {
  OK = 200,
  NOT_FOUND = 404,
  BAD_REQUEST = 400
};

void create_response(int socket_fd, unsigned status);
void put_body(int socket_fd, char *path, unsigned status_code);