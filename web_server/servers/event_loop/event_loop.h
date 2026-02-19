#ifndef EVENT_LOOP_H
#define EVENT_LOOP_H

#include <sys/epoll.h>
#include <stdint.h>

#define MAX_EVENTS 64

typedef struct event_loop {
    int epoll_fd;
    struct epoll_event events[MAX_EVENTS];
    int (*on_accept)(struct event_loop* loop, int fd);
    int (*on_read)(struct event_loop* loop, int fd);
    int (*on_write)(struct event_loop* loop, int fd);
    void* data;
} event_loop_t;

void event_loop_init(event_loop_t* loop);
void event_loop_set_callbacks(event_loop_t* loop, int (*on_accept)(event_loop_t*, int), int (*on_read)(event_loop_t*, int), int (*on_write)(event_loop_t*, int));
void event_loop_add_fd(event_loop_t* loop, int fd, uint32_t events);
void event_loop_mod_fd(event_loop_t* loop, int fd, uint32_t events);
void event_loop_del_fd(event_loop_t* loop, int fd);
void event_loop_run(event_loop_t* loop);

#endif