/*
 * Thread creation and yielding test
 *
 * Tests the creation of multiples threads and the fact that a parent thread
 * should get returned to before its child is executed. The way the printing,
 * thread creation and yielding is done, the program should output:
 *
 * thread2
 * thread1
 * thread2 returns 2
 * thread4
 * thread3
 * thread4 returns 4
 * thread1 returns 1
 */

#include <stdio.h>
#include <stdlib.h>

#include <uthread.h>

int thread4(void)
{
    uthread_yield();
    printf("thread%d\n", uthread_self());
    return 4;
}

int thread3(void)
{
    int a = 0;
    uthread_join(uthread_create(thread4), &a);
	printf("thread%d\n", uthread_self());
    printf("thread4 returns %d\n", a);
    return 3;
}

int thread2(void)
{
    uthread_create(thread3);
	uthread_yield();
	printf("thread%d\n", uthread_self());
	return 2;
}

int thread1(void)
{
    int a = 0;
    uthread_join(uthread_create(thread2), &a);
	printf("thread%d\n", uthread_self());
    printf("thread2 returns %d\n", a);
    uthread_yield();
	uthread_yield();
	return 1;
}

int main(void)
{
	uthread_start(0);
	int a = 0;
	uthread_join(uthread_create(thread1), &a);
    printf("thread1 returns %d\n", a);

    uthread_stop();

	return 0;
}
