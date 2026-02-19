#ifndef SERVER_H
#define SERVER_H

#include "../../common/include/router.h"
#include "event_loop.h"

typedef struct client {
    int fd;
    char buffer[4096];
    size_t offset;
    size_t to_send;
    char* response;
} client_t;

typedef struct {
    int port;
    int listen_fd;
    router_t router;
    event_loop_t loop;
    client_t* clients[1024];
} server_t;

void server_init(server_t* server, int port);
void server_set_routes(server_t* server);
void server_start(server_t* server);
void server_cleanup(server_t* server);

#endif