#include "../include/connection.h"
#include <unistd.h>

void connection_init(connection_t* conn, int fd) {
    conn->fd = fd;
    conn->addr_len = sizeof(conn->addr);
}

int connection_read(connection_t* conn, char* buffer, size_t buf_size) {
    return read(conn->fd, buffer, buf_size - 1);
}

int connection_write(connection_t* conn, const char* data, size_t len) {
    return write(conn->fd, data, len);
}

void connection_close(connection_t* conn) {
    if (conn->fd >= 0) {
        close(conn->fd);
        conn->fd = -1;
    }
}