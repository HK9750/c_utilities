#include "../include/demo_routes.h"
#include "../include/request.h"
#include "../include/response.h"
#include <string.h>

static void set_json_response(http_response_t* res, const char* body) {
    http_response_add_header(res, "Content-Type", "application/json");
    http_response_set_body(res, body, strlen(body));
}

static void handle_things_get(http_request_t* req, http_response_t* res) {
    (void)req;
    set_json_response(res, "{\"things\":[{\"id\":1,\"name\":\"example thing\"}]}");
}

static void handle_things_post(http_request_t* req, http_response_t* res) {
    (void)req;
    set_json_response(res, "{\"message\":\"thing created\",\"id\":2}");
}

static void handle_things_put(http_request_t* req, http_response_t* res) {
    (void)req;
    set_json_response(res, "{\"message\":\"thing replaced\",\"id\":1}");
}

static void handle_things_patch(http_request_t* req, http_response_t* res) {
    (void)req;
    set_json_response(res, "{\"message\":\"thing partially updated\",\"id\":1}");
}

static void handle_things_delete(http_request_t* req, http_response_t* res) {
    (void)req;
    set_json_response(res, "{\"message\":\"thing deleted\",\"id\":1}");
}

void demo_routes_register(router_t* router) {
    router_add_route(router, HTTP_GET, "/things", handle_things_get);
    router_add_route(router, HTTP_POST, "/things", handle_things_post);
    router_add_route(router, HTTP_PUT, "/things", handle_things_put);
    router_add_route(router, HTTP_PATCH, "/things", handle_things_patch);
    router_add_route(router, HTTP_DELETE, "/things", handle_things_delete);
}
