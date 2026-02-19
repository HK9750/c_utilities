#include "server.h"
#include "../../common/include/connection.h"
#include "../../common/include/request.h"
#include "../../common/include/response.h"
#include "../../common/include/utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

/* FIX: Max concurrent connections to prevent thread exhaustion.
 * Without a limit, the server spawns unbounded threads under load
 * until the OS refuses, causing cascading failures. */
#define MAX_CONNECTIONS 128

/* FIX: Struct to pass both the connection and server pointer to each
 * worker thread. Previously the code used an invalid cast of pthread_self()
 * to pthread_key_t* — which is undefined behavior and always crashes.
 * The idiomatic approach is to bundle everything the thread needs into
 * a heap-allocated struct passed via the thread start argument. */
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
    /* FIX: Unpack the thread_arg_t to get both the connection and server.
     * Previously: `server_t* server = (server_t*)pthread_getspecific(*(pthread_key_t*)pthread_self());`
     * which was undefined behavior — pthread_self() returns a thread ID, NOT
     * a pointer to a pthread_key_t. No key was ever created or set. */
    thread_arg_t* targ = (thread_arg_t*)arg;
    connection_t* conn = targ->conn;
    server_t* server = targ->server;
    free(targ); /* Free the arg struct now that we've extracted the pointers */

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

        free(res_data);
        http_response_free(&res);
        http_request_free(&req);
    }
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

    /* NOTE: This is an infinite loop — server_cleanup() in main.c is
     * unreachable. A proper shutdown would require a signal handler
     * (e.g. SIGINT/SIGTERM) to set a flag and break this loop. */
    while (1) {
        connection_t* conn = malloc(sizeof(connection_t));
        if (!conn) {
            perror("malloc");
            continue;
        }
        conn->addr_len = sizeof(conn->addr);
        conn->fd = accept(listen_fd, (struct sockaddr*)&conn->addr, &conn->addr_len);
        if (conn->fd < 0) {
            perror("accept");
            free(conn);
            continue;
        }

        /* FIX: Allocate a thread_arg_t to pass both conn and server to the
         * worker thread. This replaces the broken pthread_getspecific approach. */
        thread_arg_t* targ = malloc(sizeof(thread_arg_t));
        if (!targ) {
            perror("thread_arg malloc");
            connection_close(conn);
            free(conn);
            continue;
        }
        targ->conn = conn;
        targ->server = server;

        pthread_t thread;
        if (pthread_create(&thread, NULL, handle_connection, targ) != 0) {
            perror("pthread_create");
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