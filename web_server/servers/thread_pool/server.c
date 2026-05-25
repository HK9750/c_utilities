// servers/thread_pool/server.c
#include "server.h"
#include "../../common/include/connection.h"
#include "../../common/include/request.h"
#include "../../common/include/response.h"
#include "../../common/include/utils.h"
#include "../../common/include/demo_routes.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

static server_t* global_server = NULL;

static void handle_index(http_request_t* req, http_response_t* res) {
    size_t size;
    char* content = utils_read_file("../../www/index.html", &size);
    if (content) {
        http_response_set_body(res, content, size);
        http_response_add_header(res, "Content-Type", "text/html");
        free(content);
    } else {
        http_response_set_status(res, HTTP_404_NOT_FOUND);
        http_response_set_body(res, "Not Found", 9);
    }
}

static void handle_not_found(http_request_t* req, http_response_t* res) {
    http_response_set_status(res, HTTP_404_NOT_FOUND);
    http_response_set_body(res, "Not Found", 9);
}

static void process_connection(void* arg) {
    connection_t* conn = (connection_t*)arg;
    char buffer[4096];
    int n = connection_read(conn, buffer, sizeof(buffer));
    if (n > 0) {
        buffer[n] = '\0';
        http_request_t req;
        http_request_init(&req);
        http_request_parse(&req, buffer);

        http_response_t res;
        http_response_init(&res);
        if (!router_route(&global_server->router, &req, &res)) {
            handle_not_found(&req, &res);
        }

        size_t res_len;
        char* res_data = http_response_build(&res, &res_len);
        connection_write(conn, res_data, res_len);

        free(res_data);
        http_response_free(&res);
        http_request_free(&req);
    }
    connection_close(conn);
    free(conn);
}

void server_init(server_t* server, int port) {
    server->port = port;
    server->backlog = 10;
    router_init(&server->router);
    thread_pool_init(&server->pool, 4);
    global_server = server;
}

void server_set_routes(server_t* server) {
    router_add_route(&server->router, HTTP_GET, "/", handle_index);
    demo_routes_register(&server->router);
}

void server_start(server_t* server) {
    int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd < 0) {
        perror("socket");
        exit(1);
    }
    int opt = 1;
    setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(server->port);
    addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(listen_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("bind");
        close(listen_fd);
        exit(1);
    }
    if (listen(listen_fd, server->backlog) < 0) {
        perror("listen");
        close(listen_fd);
        exit(1);
    }

    printf("Server listening on port %d\n", server->port);

    while (1) {
        connection_t* conn = malloc(sizeof(connection_t));
        conn->addr_len = sizeof(conn->addr);
        conn->fd = accept(listen_fd, (struct sockaddr*)&conn->addr, &conn->addr_len);
        if (conn->fd < 0) {
            perror("accept");
            free(conn);
            continue;
        }
        thread_pool_submit(&server->pool, process_connection, conn);
    }
    close(listen_fd);
}

void server_cleanup(server_t* server) {
    thread_pool_destroy(&server->pool);
    router_free(&server->router);
}
