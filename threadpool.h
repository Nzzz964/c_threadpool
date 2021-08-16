#ifndef _N_THREADPOOL_H
#define _N_THREADPOOL_H

#include <pthread.h>
#include <stddef.h>

typedef struct task_t
{
    void (*func)(void *);
    void *argument;
} task_t;

typedef struct queue_t
{
    /** size of task */
    int size;
    /** readied task count */
    int count;
    /** head index */
    int head;
    /** multithread safe */
    pthread_mutex_t mutex;
    /** task struct array */
    task_t *task;
} queue_t;

typedef enum
{
    threadpool_status_running = 0,
    threadpool_status_shutdown = -1
} threadpool_status_t;

typedef struct threadpool_t
{
    /** persist thread num */
    int thread_count;
    threadpool_status_t status;
    /** multithread safe */
    pthread_mutex_t mutex;
    pthread_cond_t notify;
    pthread_t *threads;
    /** task queue */
    queue_t *queue;
} threadpool_t;

/** return NULL if err */
static queue_t *queue_create(int size);
static void queue_destory(queue_t *q);

/** return negative int val if err */
static int enqueue(queue_t *q, void (*func)(void *), void *argument);
/** return NULL if err */
static task_t *dequeue(queue_t *q);

threadpool_t *threadpool_create(int thread_count, queue_t *q);
void threadpool_destory(threadpool_t *pool);
static void handler(void *pool);

#endif