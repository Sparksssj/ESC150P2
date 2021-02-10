/*
 * Thread creation and yielding test
 *
 * Tests the creation of multiples threads and the fact that a parent thread
 * should get returned to before its child is executed. The way the printing,
 * thread creation and yielding is done, the program should output:
 *
 * thread1
 * thread2
 * thread3
 */

#include <stdio.h>
#include <stdlib.h>

#include "uthread.h"

char a;
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
    while (a);
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
    int ptr;
    scanf("%c", &a);
	uthread_start(1);
	uthread_join(uthread_create(thread1), &ptr);
    printf("0 thread%d  return value:%d\n", uthread_self(),ptr);
	uthread_stop();
	return 0;
}
