#include <stdio.h>
#include <stdlib.h>

#include <uthread.h>
#include "preempt.c"

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
    while(1);
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

void preempt_stop_test(void)
{
    uthread_start(1);
    preempt_stop();
    uthread_join(uthread_create(thread1), NULL);
    uthread_stop();
}

int main(void)
{
    fprintf(stderr, "*** TEST preempt stop ***\n");
    fprintf(stderr, "*** should print thread 1 then stuck into infinity loop ***\n");
    preempt_stop_test();
    return 0;
}
