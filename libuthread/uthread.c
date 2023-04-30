#include <assert.h>
#include <limits.h>
#include <signal.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

#include "private.h"
#include "uthread.h"
#include "queue.h"


uthread_t id = 0;

struct TCB * mainThread;
struct TCB * previousThread;
struct TCB * currentThread;

queue_t threadQueue,readyQueue;

struct TCB
{
	uthread_t TID;
	uthread_ctx_t uctx;
	void *stackp;
	char* state;
	int ret;
};


int uthread_start(int preempt)
{	
	if(preempt){
		preempt_start();
	}
	preempt_disable();
	threadQueue = queue_create();
	readyQueue = queue_create();
	preempt_enable();
	mainThread = (struct TCB*)malloc(sizeof(struct TCB));
	if (mainThread == NULL)
	{
		fprintf(stderr, "unable to allocate memory.\n");
		return -1;
	}
	mainThread->TID = id;
	mainThread->stackp =  uthread_ctx_alloc_stack();
	uthread_ctx_init(&mainThread->uctx, mainThread->stackp , NULL);
	if(mainThread->stackp ==NULL){
		return -1;
	}
	currentThread = mainThread;
	return 0;
}

int uthread_stop(void)
{
	// check if the one in queue is main thread
	if(queue_length(readyQueue) != 0)
		return -1;
	free(mainThread);
	return 0;
}

int uthread_create(uthread_func_t func)
{
	struct TCB *thread = (struct TCB*)malloc(sizeof(struct TCB));
	if(id + 1 > USHRT_MAX){
		return -1;
	}
	id++;
	thread->TID = id;
	thread->state= "ready";
	thread->stackp = uthread_ctx_alloc_stack();
	if(thread->stackp == NULL)
	{
		fprintf(stderr, "unable to allocate memory.\n");
		return -1;
	}
	if(uthread_ctx_init(&thread->uctx, thread->stackp, func)){
		return -1;
	}
	preempt_disable();
	queue_enqueue(threadQueue, thread);
	queue_enqueue(readyQueue, thread);
	preempt_enable();
	return thread->TID;
}

void uthread_yield(void)
{	
	previousThread = currentThread;
	previousThread -> state = "ready";
	struct TCB * nextThread = (struct TCB*)malloc(sizeof(struct TCB));
	preempt_disable();
	queue_dequeue(threadQueue, (void**)&nextThread);
	queue_enqueue(threadQueue, nextThread);
	while(strcmp(nextThread-> state, "ready")){
		queue_dequeue(threadQueue, (void**)&nextThread);
		queue_enqueue(threadQueue, nextThread);
	}
	preempt_enable();
	currentThread = nextThread;
	currentThread -> state = "running";
	if(previousThread == currentThread)
	{
		queue_delete(readyQueue, currentThread);
	}else
	{
		uthread_ctx_switch(&previousThread->uctx, &currentThread->uctx);
	}
}

uthread_t uthread_self(void)
{
	return currentThread->TID;
}

void uthread_exit(int retval)
{
	previousThread = currentThread;
	previousThread -> state = "zombie";
	previousThread -> ret = retval;
	queue_delete(readyQueue, currentThread);
	currentThread = mainThread;
	uthread_ctx_switch(&previousThread->uctx, &mainThread->uctx);
}

static int find_tcb(queue_t q, void *data, void *arg)
{
    struct TCB* a = (struct TCB*)data;
    uthread_t match = *((uthread_t*) arg);
    (void)q;
    if (a-> TID == match)
        return 1;
    return 0;
}


int uthread_join(uthread_t tid, int *retval)
{	
	if(tid == 0)
		return -1;
	currentThread -> state = "blocked";
	queue_delete(threadQueue, currentThread);
	struct TCB * childThread = (struct TCB*)malloc(sizeof(struct TCB));
	void * ptr = malloc(sizeof(uthread_t));
	*((uthread_t*)ptr) = tid;
	queue_iterate(threadQueue, find_tcb, ptr, (void**) &childThread);
	free(ptr);
	while(strcmp(childThread-> state, "zombie")){
		uthread_yield();
	}
	if(retval != NULL){
		*retval = childThread-> ret;
	}
	queue_delete(threadQueue, childThread);
	free(childThread);
	currentThread -> state = "ready";
	preempt_disable();
	queue_enqueue(threadQueue, currentThread);
	preempt_enable();
	while(queue_length(readyQueue) != 0){
		uthread_yield();
	}
	preempt_disable();
	queue_enqueue(readyQueue, currentThread);
	preempt_enable();
	return -1;
}
