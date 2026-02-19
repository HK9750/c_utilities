#include "server.h"

int main() {
    server_t server;
    server_init(&server, 8080);
    server_set_routes(&server);
    server_start(&server);
    server_cleanup(&server);
    return 0;
}