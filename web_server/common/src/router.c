#include "../include/router.h"
#include "../include/logger.h"
#include <stdlib.h>
#include <string.h>

void router_init(router_t* router) {
    router->routes = NULL;
}

void router_add_route(router_t* router, http_method_t method, const char* path, route_handler_t handler) {
    route_entry_t* entry = malloc(sizeof(route_entry_t));
    if (!entry) {
        LOG_ERROR("malloc failed for route entry");
        return;
    }
    entry->path = strdup(path);
    entry->method = method;
    entry->handler = handler;
    entry->next = router->routes;
    router->routes = entry;
    (void)handler;
    LOG_DEBUG("registered route: %s %s", http_method_str(method), path);
}

int router_route(router_t* router, http_request_t* req, http_response_t* res) {
    for (route_entry_t* r = router->routes; r != NULL; r = r->next) {
        if (r->method == req->method && strcmp(r->path, req->path) == 0) {
            LOG_DEBUG("route matched: %s %s", http_method_str(req->method), req->path);
            r->handler(req, res);
            return 1;
        }
    }
    LOG_DEBUG("no route matched: %s %s", http_method_str(req->method), req->path ? req->path : "(null)");
    return 0;
}

void router_free(router_t* router) {
    route_entry_t* r = router->routes;
    while (r) {
        route_entry_t* next = r->next;
        free(r->path);
        free(r);
        r = next;
    }
}