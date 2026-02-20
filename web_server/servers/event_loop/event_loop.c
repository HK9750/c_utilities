#include "event_loop.h"
#include <stdio.h>
#include <unistd.h>

// epoll_create1 is a system call that creates an epoll instance and returns
//  a file descriptor referring to that instance. 
// The argument flags can be 0 or EPOLL_CLOEXEC.
//  If flags is 0, the file descriptor returned by epoll_create1 will be created with the O_CLOEXEC flag,
//  which means it will be automatically closed during an execve() system call.
//  If flags is EPOLL_CLOEXEC, the file descriptor will not have the O_CLOEXEC flag set,
//  and it will not be automatically closed during an execve() system call.

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
    // epoll_event is a structure that describes an event that can be monitored by epoll. It has two members: events and data.
    struct epoll_event ev;
    ev.events = events;
    ev.data.fd = fd;
    // epoll_ctl is a system call that controls the epoll instance referred to by the file descriptor epfd. 
    // It performs the specified operation (op) on the target file descriptor fd, 
    // and the event parameter is used to specify the events to be monitored for that file descriptor.
    epoll_ctl(loop->epoll_fd, EPOLL_CTL_ADD, fd, &ev);
}

void event_loop_mod_fd(event_loop_t* loop, int fd, uint32_t events) {
    struct epoll_event ev;
    ev.events = events;
    ev.data.fd = fd;
    epoll_ctl(loop->epoll_fd, EPOLL_CTL_MOD, fd, &ev);
}

void event_loop_del_fd(event_loop_t* loop, int fd) {
    // When deleting a file descriptor from the epoll instance, we can pass NULL for the event parameter since it's not needed for deletion
    epoll_ctl(loop->epoll_fd, EPOLL_CTL_DEL, fd, NULL);
}

void event_loop_run(event_loop_t* loop) {
    while (1) {
        // epollwait takes in parameters: epoll file descriptor, array to store events, max events, timeout
        // it returns the number of file descriptors that are ready for the requested I/O, or -1 on error
        int nfds = epoll_wait(loop->epoll_fd, loop->events, MAX_EVENTS, -1);
        if (nfds < 0) {
            perror("epoll_wait returned -1");
            break;
        }
        for (int i = 0; i < nfds; i++) {
            // The file descriptor that is ready for I/O is stored in loop->events[i].data.fd
            int fd = loop->events[i].data.fd;
            // if the event is EPOLLIN, it means the file descriptor is ready for reading
            // Macro EPOLLIN is defined in sys/epoll.h and indicates that the associated file descriptor is available for read operations
            if (loop->events[i].events & EPOLLIN) {
                // If the on_read callback is set, call it with the event loop and the file descriptor
                if (loop->on_read && loop->on_read(loop, fd) < 0) {
                    close(fd);
                    event_loop_del_fd(loop, fd);
                }
            }
            // if the event is EPOLLOUT, it means the file descriptor is ready for writing
            // Macro EPOLLOUT is defined in sys/epoll.h and indicates that the associated file descriptor is available for write operations
            if (loop->events[i].events & EPOLLOUT) {
                if (loop->on_write && loop->on_write(loop, fd) < 0) {
                    close(fd);
                    event_loop_del_fd(loop, fd);
                }
            }
        }
    }
}