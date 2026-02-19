#include "../include/connection.h"
#include "../include/logger.h"
#include <unistd.h>

void connection_init(connection_t* conn, int fd) {
    conn->fd = fd;
    conn->addr_len = sizeof(conn->addr);
}

int connection_read(connection_t* conn, char* buffer, size_t buf_size) {
    if (conn->fd < 0) {
        LOG_ERROR("read on invalid fd");
        return -1;
    }
    int n = read(conn->fd, buffer, buf_size - 1);
    if (n < 0) LOG_ERROR("read failed on fd %d", conn->fd);
    return n;
}

int connection_write(connection_t* conn, const char* data, size_t len) {
    if (conn->fd < 0) {
        LOG_ERROR("write on invalid fd");
        return -1;
    }
    int n = write(conn->fd, data, len);
    if (n < 0) LOG_ERROR("write failed on fd %d", conn->fd);
    return n;
}

void connection_close(connection_t* conn) {
    if (conn->fd >= 0) {
        LOG_DEBUG("closing fd %d", conn->fd);
        close(conn->fd);
        conn->fd = -1;
    }
}