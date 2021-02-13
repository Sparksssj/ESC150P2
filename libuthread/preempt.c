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

void termination_handler ()
{
    // When timer reached, call yield to next ready thread.
    uthread_yield();
}

void preempt_start(void)
{
	signal_handler.sa_handler = termination_handler
    // Set the mask to include SIGVTALRM only
    sigemptyset(&signal_sets);
    sigaddset(&signal_sets, SIGVTALRM);
    signal_handler.sa_mask = signal_sets;
    // Set the signal handler
    sigaction(SIGVTALRM, &signal_handler, &signal_handler_restore);
    // Set the time between now and the first interrupt by 10hz
	timer.it_interval.tv_sec = 0;
	timer.it_interval.tv_usec = 1000/HZ;
	// Set the time between successive interrupt by 10hz
	timer.it_value.tv_sec = 0;
	timer.it_value.tv_usec = 1000/HZ;
	// Set the alarm
	setitimer(ITIMER_VIRTUAL, &timer, &timer_restore);
}

void preempt_stop(void)
{
    // Disable the alarm/timer
    timer.it_interval.tv_sec = 0;
    timer.it_interval.tv_usec = 0;
    timer.it_value.tv_sec = 0;
    timer.it_value.tv_usec = 0;
    setitimer(ITIMER_VIRTUAL, &timer, NULL);
    // Restore previous timer, signal handler configuration
    signal_handler = signal_handler_restore;
    timer = timer_restore;
}

void preempt_enable(void)
{
    // unblock the mask
	sigprocmask(SIG_UNBLOCK, &signal_sets, NULL);
}

void preempt_disable(void)
{
	// block the mask
    sigprocmask(SIG_BLOCK, &signal_sets, NULL);
}

