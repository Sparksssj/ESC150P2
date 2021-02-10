#include <signal.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#include "private.h"
#include "uthread.h"

/*
 * Frequency of preemption
 * 100Hz is 100 times per second
 */
#define HZ 100

struct itimerval timer, timer_restore;
struct sigaction signal_handler, signal_handler_restore;
sigset_t signal_sets;

void termination_handler (int signum)
{
    printf("a\n");
    uthread_yield();
}

void preempt_start(void)
{
	/* TODO */
	signal_handler.sa_handler = termination_handler;
    sigemptyset(&signal_sets);
    sigaddset(&signal_sets, SIGVTALRM);
    signal_handler.sa_mask = signal_sets;
    sigaction(SIGVTALRM, &signal_handler, &signal_handler_restore);
	timer.it_interval.tv_sec = 0;
	timer.it_interval.tv_usec = 1000/HZ;
	timer.it_value.tv_sec = 0;
	timer.it_value.tv_usec = 1000/HZ;
	setitimer(ITIMER_VIRTUAL, &timer, &timer_restore);
}

void preempt_stop(void)
{
	/* TODO */
	timer.it_interval.tv_sec = 0;
    timer.it_interval.tv_usec = 0;
    timer.it_value.tv_sec = 0;
    timer.it_value.tv_usec = 0;
}

void preempt_enable(void)
{
	/* TODO */
	sigprocmask(SIG_UNBLOCK, &signal_sets, NULL);
}

void preempt_disable(void)
{
	/* TODO */
    sigprocmask(SIG_BLOCK, &signal_sets, NULL);
}

