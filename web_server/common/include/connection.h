#ifndef CONNECTION_H
#define CONNECTION_H

#include <netinet/in.h>

typedef struct {
    int fd;
    struct sockaddr_in addr;
    socklen_t addr_len;
} connection_t;

void connection_init(connection_t* conn, int fd);
int connection_read(connection_t* conn, char* buffer, size_t buf_size);
int connection_write(connection_t* conn, const char* data, size_t len);
void connection_close(connection_t* conn);

#endif