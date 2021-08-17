#include <stdio.h>
#include <unistd.h>
#include <memory.h>
#include <stdlib.h>
#include "threadpool.h"

void *func(void *argument)
{
    int *num = (int *)argument;
    printf("func: %d\n", *num);
    return NULL;
}

int main()
{
    int count = 1024;
    int *args = (int *)malloc(sizeof(int) * count);

    queue_t *queue = queue_create(1024);
    threadpool_t *pool = threadpool_create(64, queue);
    for (int i = 0; i < count; i++)
    {
        const void *tmp = &i;
        void *arg = (void *)(&args[i]);
        memcpy(arg, tmp, sizeof(int));
        threadpool_add(pool, func, arg);
    }
    threadpool_destory(pool);
    queue_destory(queue);
    free(args);
    return 0;
}