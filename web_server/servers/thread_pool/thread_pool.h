#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <pthread.h>

typedef struct task {
    void (*function)(void*);
    void* arg;
    struct task* next;
} task_t;

typedef struct {
    pthread_t* threads;
    int thread_count;
    task_t* task_queue;
    pthread_mutex_t queue_mutex;
    pthread_cond_t queue_cond;
    int stop;
} thread_pool_t;

void thread_pool_init(thread_pool_t* pool, int num_threads);
void thread_pool_submit(thread_pool_t* pool, void (*func)(void*), void* arg);
void thread_pool_destroy(thread_pool_t* pool);

#endif