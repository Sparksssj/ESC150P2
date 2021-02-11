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



int thread5(void)
{
    printf("5 thread%d\n", uthread_self());
    return 5;
}

int thread4(void)
{
    int c = 0;
    int* ptr = &c;
    uthread_join(2, ptr);
    uthread_create(thread5);
    uthread_yield();
    printf("4 thread%d  return value:%d\n", uthread_self(),*ptr);
    return 4;
}

int thread3(void)
{
    int b = 0;
    int* ptr = &b;
    uthread_join(uthread_create(thread4), ptr);
    printf("3 thread%d  return value:%d\n", uthread_self(),*ptr);
    return 3;
}

int thread2(void)
{
    uthread_create(thread3);
    uthread_yield();
    printf("2 thread%d\n", uthread_self());
    return 2;
}

int thread1(void)
{
    uthread_create(thread2);
    uthread_yield();
    printf("1 thread%d\n", uthread_self());
    return 1;
}