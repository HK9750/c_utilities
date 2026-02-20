// servers/event_loop/server.c
#include "server.h"
#include "../../common/include/request.h"
#include "../../common/include/response.h"
#include "../../common/include/utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>

// set_nonblock is a helper function that sets a file descriptor to non-blocking mode using fcntl system call
static void set_nonblock(int fd) {
    // fcntl is a system call that performs various operations on a file descriptor. 
    // F_GETFL is used to get the file status flags, and F_SETFL is used to set the file status flags.
    //  O_NONBLOCK is a flag that makes the file descriptor non-blocking,
    //  meaning that I/O operations will return immediately instead of blocking if they cannot be completed immediately.
    int flags = fcntl(fd, F_GETFL, 0);
    fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

static int on_accept(event_loop_t* loop, int listen_fd) {
    server_t* server = (server_t*)loop->data;
    struct sockaddr_in addr;
    socklen_t addr_len = sizeof(addr);
    int client_fd = accept(listen_fd, (struct sockaddr*)&addr, &addr_len);
    if (client_fd < 0) return -1;
    set_nonblock(client_fd);

    client_t* client =(client_t *)malloc(sizeof(client_t));
    client->fd = client_fd;
    client->offset = 0;
    client->to_send = 0;
    client->response = NULL;
    memset(client->buffer, 0, sizeof(client->buffer));
    server->clients[client_fd] = client;

    event_loop_add_fd(loop, client_fd, EPOLLIN | EPOLLET);
    return 0;
}

static int on_read(event_loop_t* loop, int fd) {
    server_t* server = (server_t*)loop->data;
    client_t* client = server->clients[fd];
    if (!client) return -1;

    int n = read(fd, client->buffer + client->offset, sizeof(client->buffer) - client->offset - 1);
    if (n <= 0) {
        if (n == 0 || (n < 0 && errno != EAGAIN)) {
            free(client);
            server->clients[fd] = NULL;
            return -1;
        }
        return 0;
    }
    client->offset += n;
    client->buffer[client->offset] = '\0';

    char* end = strstr(client->buffer, "\r\n\r\n");
    if (!end) return 0;

    http_request_t req;
    http_request_init(&req);
    http_request_parse(&req, client->buffer);

    http_response_t res;
    http_response_init(&res);
    if (!router_route(&server->router, &req, &res)) {
        http_response_set_status(&res, HTTP_404_NOT_FOUND);
        http_response_set_body(&res, "Not Found", 9);
    }

    size_t res_len;
    client->response = http_response_build(&res, &res_len);
    client->to_send = res_len;

    http_response_free(&res);
    http_request_free(&req);

    event_loop_mod_fd(loop, fd, EPOLLOUT | EPOLLET);
    return 0;
}

static int on_write(event_loop_t* loop, int fd) {
    server_t* server = (server_t*)loop->data;
    client_t* client = server->clients[fd];
    if (!client || !client->response) return -1;

    int n = write(fd, client->response, client->to_send);
    if (n < 0) {
        if (errno != EAGAIN) {
            free(client->response);
            free(client);
            server->clients[fd] = NULL;
            return -1;
        }
        return 0;
    }
    client->to_send -= n;
    if (client->to_send == 0) {
        free(client->response);
        close(fd);
        free(client);
        server->clients[fd] = NULL;
        return -1;
    }
    memmove(client->response, client->response + n, client->to_send);
    return 0;
}

static void handle_index(http_request_t* req, http_response_t* res) {
    (void)req;
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

void server_init(server_t* server, int port) {
    server->port = port;
    router_init(&server->router);
    event_loop_init(&server->loop);
    server->loop.data = server;
    event_loop_set_callbacks(&server->loop, on_accept, on_read, on_write);
    memset(server->clients, 0, sizeof(server->clients));
}

void server_set_routes(server_t* server) {
    router_add_route(&server->router, HTTP_GET, "/", handle_index);
}

void server_start(server_t* server) {
    server->listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server->listen_fd < 0) {
        perror("socket");
        exit(1);
    }
    int opt = 1;
    setsockopt(server->listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    set_nonblock(server->listen_fd);

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(server->port);
    addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(server->listen_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("bind");
        close(server->listen_fd);
        exit(1);
    }
    if (listen(server->listen_fd, 10) < 0) {
        perror("listen");
        close(server->listen_fd);
        exit(1);
    }

    event_loop_add_fd(&server->loop, server->listen_fd, EPOLLIN | EPOLLET);
    printf("Server listening on port %d\n", server->port);
    event_loop_run(&server->loop);
}

void server_cleanup(server_t* server) {
    close(server->listen_fd);
    for (int i = 0; i < 1024; i++) {
        if (server->clients[i]) {
            close(i);
            free(server->clients[i]->response);
            free(server->clients[i]);
        }
    }
    router_free(&server->router);
}