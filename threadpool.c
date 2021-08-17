#include <stddef.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>

#include "threadpool.h"

static void *handler(void *threadpool);

queue_t *queue_create(int size)
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
    return queue;

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

void queue_destory(queue_t *q)
{
    pthread_mutex_destroy(&(q->mutex));
    free(q->task);
    free(q);
}

int enqueue(queue_t *q, void *(*func)(void *), void *argument)
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

task_t *dequeue(queue_t *q)
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
        if (pthread_create(&(pool->threads[i]), NULL, handler, (void *)pool) != 0)
        {
            goto err;
        }
        pool->thread_count++;
    }
    pool->status = threadpool_status_running;
    return pool;
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
    LOG("threadpool destoring");
    pthread_mutex_lock(&(pool->mutex));
    pool->status = threadpool_status_shutdown;
    pthread_cond_broadcast(&(pool->notify));
    pthread_mutex_unlock(&(pool->mutex));
    for (int i = 0; i < pool->thread_count; i++)
    {
        pthread_join(pool->threads[i], NULL);
    }
    pthread_mutex_destroy(&(pool->mutex));
    pthread_cond_destroy(&(pool->notify));
    free(pool->threads);
    free(pool);
}

void threadpool_add(threadpool_t *pool, void *(*func)(void *), void *argument)
{
    enqueue(pool->queue, func, argument);
    pthread_cond_signal(&(pool->notify));
}

static void *handler(void *threadpool)
{
    threadpool_t *pool = (threadpool_t *)threadpool;
    for (;;)
    {
        pthread_mutex_lock(&(pool->mutex));

        while (pool->queue->count == 0 && pool->status != threadpool_status_shutdown)
        {
            LOG("pthread_cond_wait");
            pthread_cond_wait(&(pool->notify), &(pool->mutex));
        }

        //结束进程池 要先完成队列里的所有任务
        if (pool->queue->count == 0 && pool->status == threadpool_status_shutdown)
        {
            break;
        }

        task_t *task = dequeue(pool->queue);
        pthread_mutex_unlock(&(pool->mutex));

        (task->func)(task->argument);
    }

    pthread_mutex_unlock(&(pool->mutex));
    LOG("pthread_exit");
    pthread_exit(NULL);
}
