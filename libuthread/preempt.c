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

void timer_handler(){
	uthread_yield();
}

void preempt_start(void)
{
	/* TODO */
	struct sigaction sa;
	struct itimerval interval;

	int timer=1000000/HZ;

	sa.sa_handler = timer_handler;
	sigemptyset(&sa.sa_mask);

	if (sigaction(SIGVTALRM, &sa, NULL)) {
		exit(1);
	}

	interval.it_value.tv_sec=0;
	interval.it_value.tv_usec= timer;
	interval.it_interval.tv_sec=0;
	interval.it_interval.tv_usec= timer;
	if(setitimer(ITIMER_VIRTUAL, &interval, NULL)){
		exit(1);
	}
}

void preempt_stop(void)
{
	/* TODO */
	sigset_t signal;
	sigprocmask(SIG_SETMASK,&signal,NULL);
}

void preempt_enable(void)
{
	/* TODO */
	sigset_t signal;
	sigemptyset(&signal);
	sigaddset(&signal, SIGVTALRM);
	//unblock the alarm signal
	sigprocmask(SIG_UNBLOCK, &signal, NULL);
}

void preempt_disable(void)
{
	/* TODO */
	sigset_t signal;
	sigemptyset(&signal);
	sigaddset(&signal, SIGVTALRM);
	//block the alarm signal
	sigprocmask(SIG_BLOCK, &signal, NULL);
}



