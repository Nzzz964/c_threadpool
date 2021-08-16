#include <stddef.h>
#include <pthread.h>
#include <stdlib.h>

#include "threadpool.h"

static queue_t *queue_create(int size)
{
    queue_t *queue = (queue_t *)malloc(sizeof(queue_t) * size);
    if (queue == NULL)
    {
        goto err;
    }
    queue->size = size;
    queue->head = 0;
    queue->count = 0;

    if (pthread_mutex_init(&(queue->mutex), NULL) != 0)
    {
        goto err;
    }

    queue->task = (task_t *)malloc(sizeof(task_t) * size);
    if (queue->task == NULL)
    {
        goto err;
    }
    return &queue;

err:
    if (queue->task != NULL)
    {
        free(queue->task);
    }
    if (queue != NULL)
    {
        free(queue);
    }
    return NULL;
}

static void queue_destory(queue_t *q)
{
    pthread_mutex_destroy(&(q->mutex));
    free(q->task);
    free(q);
}

static int enqueue(queue_t *q, void (*func)(void *), void *argument)
{
    pthread_mutex_lock(&(q->mutex));

    if (q->count > q->size)
    {
        return -1;
    }
    int pos = (q->head + q->count) % q->size;
    q->task[pos].func = func;
    q->task[pos].argument = argument;
    q->count += 1;

    pthread_mutex_unlock(&(q->mutex));
    return 0;
}

static task_t *dequeue(queue_t *q)
{
    pthread_mutex_lock(&(q->mutex));
    if (q->count == 0)
    {
        return NULL;
    }
    task_t *task = &(q->task[q->head]);
    q->head = (q->head + 1) % q->size;
    q->count -= 1;
    pthread_mutex_unlock(&(q->mutex));
    return task;
}

threadpool_t *threadpool_create(int thread_count, queue_t *q)
{
    threadpool_t *pool = (threadpool_t *)malloc(sizeof(threadpool_t));
    if (pool == NULL)
    {
        goto err;
    }
    pool->queue = q;

    if (pthread_mutex_init(&(pool->mutex), NULL) != 0 ||
        pthread_cond_init(&(pool->notify), NULL) != 0)
    {
        goto err;
    };

    pool->threads = (pthread_t *)malloc(sizeof(pthread_t) * thread_count);
    if (pool->threads == NULL)
    {
        goto err;
    }

    pool->thread_count = 0;
    for (int i = 0; i < thread_count; i++)
    {
        if (pthread_create(pool->threads[i], NULL, handler, (void *)pool) != 0)
        {
            goto err;
        }
        pool->thread_count++;
    }
    pool->status = threadpool_status_running;
err:
    if (pool->thread_count != 0)
    {
        for (int i = 0; i < pool->thread_count; i++)
        {
            pthread_cancel(pool->threads[i]);
        }
    }
    if (pool->threads != NULL)
    {
        free(pool->threads);
    }
    if (pool != NULL)
    {
        free(pool);
    }
    return NULL;
}

void threadpool_destory(threadpool_t *pool)
{
}

static void handler(void *pool)
{
    (threadpool_t *)pool;
}
