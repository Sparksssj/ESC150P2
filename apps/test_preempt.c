#include <stdio.h>
#include <stdlib.h>

#include <uthread.h>
#include "preempt.c"

int thread9(void)
{
    uthread_yield();
    printf("thread%d\n", uthread_self());
    return 0;
}

int thread8(void)
{
    uthread_create(thread9);
    uthread_yield();
    preempt_disable();
    while(1);
    printf("thread%d\n", uthread_self());
    return 0;
}

int thread7(void)
{
    uthread_create(thread8);
    uthread_yield();
    printf("thread%d\n", uthread_self());
    uthread_yield();
    return 0;
}

int thread6(void)
{
    uthread_yield();
    printf("thread%d\n", uthread_self());
    return 0;
}

int thread5(void)
{
    uthread_create(thread6);
    uthread_yield();
    preempt_disable();
    preempt_enable();
    while(1);
    printf("thread%d\n", uthread_self());
    return 0;
}

int thread4(void)
{
    uthread_create(thread5);
    uthread_yield();
    printf("thread%d\n", uthread_self());
    uthread_yield();
    return 0;
}

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

void preempt_start_test(void)
{
    uthread_start(1);
    uthread_join(uthread_create(thread1), NULL);
    uthread_stop();
}

void preempt_enable_test(void)
{
    uthread_start(1);
    uthread_join(uthread_create(thread4), NULL);
    uthread_stop();
}

void preempt_disable_test(void)
{
    uthread_start(1);
    uthread_join(uthread_create(thread7), NULL);
    uthread_stop();
}

int main(void)
{
    fprintf(stderr, "*** TEST preempt ***\n");
    fprintf(stderr, "*** should print thread 1 thread 3 ***\n");
    preempt_start_test();
    fprintf(stderr, "*** TEST enable ***\n");
    fprintf(stderr, "*** should print thread 4 thread 6 ***\n");
    preempt_enable_test();
    fprintf(stderr, "*** TEST disable ***\n");
    fprintf(stderr, "*** should print thread 7 then infinite loop ***\n");
    preempt_disable_test();
    return 0;
}
