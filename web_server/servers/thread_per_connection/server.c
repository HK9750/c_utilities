#include "server.h"
#include "../../common/include/connection.h"
#include "../../common/include/request.h"
#include "../../common/include/response.h"
#include "../../common/include/utils.h"
#include "../../common/include/logger.h"
#include "../../common/include/demo_routes.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define MAX_CONNECTIONS 128

typedef struct {
    connection_t* conn;
    server_t* server;
} thread_arg_t;

static void handle_index(http_request_t* req, http_response_t* res) {
    (void)req; /* Handler signature requires req; unused here */
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
    (void)req; /* Handler signature requires req; unused here */
    http_response_set_status(res, HTTP_404_NOT_FOUND);
    http_response_set_body(res, "Not Found", 9);
}

static void* handle_connection(void* arg) {
    thread_arg_t* targ = (thread_arg_t*)arg;
    connection_t* conn = targ->conn;
    server_t* server = targ->server;
    free(targ);

    char client_ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &conn->addr.sin_addr, client_ip, sizeof(client_ip));
    int client_port = ntohs(conn->addr.sin_port);
    LOG_INFO("connection accepted from %s:%d (fd %d)", client_ip, client_port, conn->fd);

    char buffer[4096];
    int n = connection_read(conn, buffer, sizeof(buffer));
    if (n > 0) {
        buffer[n] = '\0';
        http_request_t req;
        http_request_init(&req);
        http_request_parse(&req, buffer);

        http_response_t res;
        http_response_init(&res);
        if (!router_route(&server->router, &req, &res)) {
            handle_not_found(&req, &res);
        }

        size_t res_len;
        char* res_data = http_response_build(&res, &res_len);
        connection_write(conn, res_data, res_len);

        LOG_INFO("%s %s -> %d (%zu bytes)",
                 http_method_str(req.method),
                 req.path ? req.path : "(null)",
                 res.status_code,
                 res_len);

        free(res_data);
        http_response_free(&res);
        http_request_free(&req);
    }
    LOG_DEBUG("connection from %s:%d finished", client_ip, client_port);
    connection_close(conn);
    free(conn);
    return NULL;
}

void server_init(server_t* server, int port) {
    server->port = port;
    server->backlog = 10;
    router_init(&server->router);
}

void server_set_routes(server_t* server) {
    router_add_route(&server->router, HTTP_GET, "/", handle_index);
    demo_routes_register(&server->router);
}

void server_start(server_t* server) {
    int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd < 0) {
        LOG_ERROR("socket creation failed: %s", strerror(errno));
        exit(1);
    }
    int opt = 1;
    setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(server->port);
    addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(listen_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        LOG_ERROR("bind failed on port %d: %s", server->port, strerror(errno));
        close(listen_fd);
        exit(1);
    }
    if (listen(listen_fd, server->backlog) < 0) {
        LOG_ERROR("listen failed: %s", strerror(errno));
        close(listen_fd);
        exit(1);
    }

    LOG_INFO("server listening on port %d", server->port);

    while (1) {
        connection_t* conn = (connection_t *)malloc(sizeof(connection_t));
        if (!conn) {
            LOG_ERROR("malloc failed for connection: %s", strerror(errno));
            continue;
        }
        conn->addr_len = sizeof(conn->addr);
        conn->fd = accept(listen_fd, (struct sockaddr*)&conn->addr, &conn->addr_len);
        if (conn->fd < 0) {
            LOG_ERROR("accept failed: %s", strerror(errno));
            free(conn);
            continue;
        }

        thread_arg_t* targ = (thread_arg_t *)malloc(sizeof(thread_arg_t));
        if (!targ) {
            LOG_ERROR("malloc failed for thread arg: %s", strerror(errno));
            connection_close(conn);
            free(conn);
            continue;
        }
        targ->conn = conn;
        targ->server = server;

        pthread_t thread;
        if (pthread_create(&thread, NULL, handle_connection, targ) != 0) {
            LOG_ERROR("pthread_create failed: %s", strerror(errno));
            connection_close(conn);
            free(conn);
            free(targ);
            continue;
        }
        pthread_detach(thread);
    }
    close(listen_fd);
}

void server_cleanup(server_t* server) {
    router_free(&server->router);
}
