#ifndef ROUTER_H
#define ROUTER_H

#include "request.h"
#include "response.h"

typedef void (*route_handler_t)(http_request_t*, http_response_t*);

typedef struct route_entry {
    char* path;
    http_method_t method;
    route_handler_t handler;
    struct route_entry* next;
} route_entry_t;

typedef struct {
    route_entry_t* routes;
} router_t;

void router_init(router_t* router);
void router_add_route(router_t* router, http_method_t method, const char* path, route_handler_t handler);
int router_route(router_t* router, http_request_t* req, http_response_t* res);
void router_free(router_t* router);

#endif