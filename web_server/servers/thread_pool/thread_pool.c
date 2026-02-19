#include "thread_pool.h"
#include <stdlib.h>
#include <stdio.h>

static void* worker(void* arg) {
    thread_pool_t* pool = (thread_pool_t*)arg;
    while (1) {
        pthread_mutex_lock(&pool->queue_mutex);
        while (pool->task_queue == NULL && !pool->stop) {
            pthread_cond_wait(&pool->queue_cond, &pool->queue_mutex);
        }
        if (pool->stop && pool->task_queue == NULL) {
            pthread_mutex_unlock(&pool->queue_mutex);
            break;
        }
        task_t* task = pool->task_queue;
        pool->task_queue = task->next;
        pthread_mutex_unlock(&pool->queue_mutex);
        task->function(task->arg);
        free(task);
    }
    return NULL;
}

void thread_pool_init(thread_pool_t* pool, int num_threads) {
    pool->thread_count = num_threads;
    pool->threads = malloc(sizeof(pthread_t) * num_threads);
    pool->task_queue = NULL;
    pthread_mutex_init(&pool->queue_mutex, NULL);
    pthread_cond_init(&pool->queue_cond, NULL);
    pool->stop = 0;
    for (int i = 0; i < num_threads; i++) {
        pthread_create(&pool->threads[i], NULL, worker, pool);
    }
}

void thread_pool_submit(thread_pool_t* pool, void (*func)(void*), void* arg) {
    task_t* task = malloc(sizeof(task_t));
    task->function = func;
    task->arg = arg;
    task->next = NULL;
    pthread_mutex_lock(&pool->queue_mutex);
    if (pool->task_queue == NULL) {
        pool->task_queue = task;
    } else {
        task_t* last = pool->task_queue;
        while (last->next) last = last->next;
        last->next = task;
    }
    pthread_cond_signal(&pool->queue_cond);
    pthread_mutex_unlock(&pool->queue_mutex);
}

void thread_pool_destroy(thread_pool_t* pool) {
    pthread_mutex_lock(&pool->queue_mutex);
    pool->stop = 1;
    pthread_cond_broadcast(&pool->queue_cond);
    pthread_mutex_unlock(&pool->queue_mutex);
    for (int i = 0; i < pool->thread_count; i++) {
        pthread_join(pool->threads[i], NULL);
    }
    while (pool->task_queue) {
        task_t* t = pool->task_queue;
        pool->task_queue = t->next;
        free(t);
    }
    free(pool->threads);
    pthread_mutex_destroy(&pool->queue_mutex);
    pthread_cond_destroy(&pool->queue_cond);
}