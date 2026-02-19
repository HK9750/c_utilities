#ifndef SERVER_H
#define SERVER_H

#include "../../common/include/router.h"

typedef struct {
    int port;
    int backlog;
    router_t router;
} server_t;

void server_init(server_t* server, int port);
void server_set_routes(server_t* server);
void server_start(server_t* server);
void server_cleanup(server_t* server);

#endif