#include <stdio.h>
#include <stdlib.h>

#include <uthread.h>

int thread3(void)
{
    uthread_yield();
    printf("thread%d\n", uthread_self());
    return 0;
}

int thread2(void)
{
    uthread_create(thread3);
    uthread_yield();
    printf("thread%d\n", uthread_self());
    return 0;
}

int thread1(void)
{
    uthread_create(thread2);
    uthread_yield();
    printf("thread%d\n", uthread_self());
    uthread_yield();
    return 0;
}

int main(void)
{
    uthread_start(0);
    uthread_join(uthread_create(thread1), NULL);
    uthread_stop();

    return 0;
}
