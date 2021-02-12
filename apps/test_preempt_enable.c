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
    preempt_disable();
    preempt_enable();
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

void preempt_enable_test(void)
{
    uthread_start(1);
    uthread_join(uthread_create(thread1), NULL);
    uthread_stop();
}

int main(void)
{
    fprintf(stderr, "*** TEST preempt enable ***\n");
    fprintf(stderr, "*** should print thread 1 thread 3 ***\n");
    preempt_enable_test();
    return 0;
}
