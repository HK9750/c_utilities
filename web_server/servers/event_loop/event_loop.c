#include "event_loop.h"
#include <unistd.h>

void event_loop_init(event_loop_t* loop) {
    loop->epoll_fd = epoll_create1(0);
    loop->on_accept = NULL;
    loop->on_read = NULL;
    loop->on_write = NULL;
    loop->data = NULL;
}

void event_loop_set_callbacks(event_loop_t* loop, int (*on_accept)(event_loop_t*, int), int (*on_read)(event_loop_t*, int), int (*on_write)(event_loop_t*, int)) {
    loop->on_accept = on_accept;
    loop->on_read = on_read;
    loop->on_write = on_write;
}

void event_loop_add_fd(event_loop_t* loop, int fd, uint32_t events) {
    struct epoll_event ev;
    ev.events = events;
    ev.data.fd = fd;
    epoll_ctl(loop->epoll_fd, EPOLL_CTL_ADD, fd, &ev);
}

void event_loop_mod_fd(event_loop_t* loop, int fd, uint32_t events) {
    struct epoll_event ev;
    ev.events = events;
    ev.data.fd = fd;
    epoll_ctl(loop->epoll_fd, EPOLL_CTL_MOD, fd, &ev);
}

void event_loop_del_fd(event_loop_t* loop, int fd) {
    epoll_ctl(loop->epoll_fd, EPOLL_CTL_DEL, fd, NULL);
}

void event_loop_run(event_loop_t* loop) {
    while (1) {
        int nfds = epoll_wait(loop->epoll_fd, loop->events, MAX_EVENTS, -1);
        for (int i = 0; i < nfds; i++) {
            int fd = loop->events[i].data.fd;
            if (loop->events[i].events & EPOLLIN) {
                if (loop->on_read && loop->on_read(loop, fd) < 0) {
                    close(fd);
                    event_loop_del_fd(loop, fd);
                }
            }
            if (loop->events[i].events & EPOLLOUT) {
                if (loop->on_write && loop->on_write(loop, fd) < 0) {
                    close(fd);
                    event_loop_del_fd(loop, fd);
                }
            }
        }
    }
}