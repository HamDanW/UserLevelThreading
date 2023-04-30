# Project 2 Report

## Queue

#### Data structure

- **Queue Struct**: The queue is a FIFO data structures that contains functions
  such as basic enqueue, dequeue, delete, iterate, and length. The struct queue
  contains a (void _) arr to store the (void _) items, size_t size to indicate
  last postition of the queue, and size_t oldest_pos to indicate the front
  position of the queue.

In the queue, we uses create to initialize the strct queue and returns queue_t
and we destroys the queue by freeing the queue_t and (void \*) arr within the
queue_t.

Special functions:

`queue_delete()`: Iterate in the queue until it reaches the desired data. If
desired data to delete is found, it will set the current item to equal the next
item in the queue. This would stops at the last position of the queue - 1. Then,
this function would remove the last element of the queue by setting it to NULL.

`queue_iterate()`: Iterate works by iterating the for loop from the oldest_pos
to size. when the special function returns 1, it stop the iteration and set
@data to current item if the @data is not NULL=, it will set the @data to
current item stopped.

`queue_length()`: Length works by subtracting the size and oldest pos.

## Thread

#### Data structure

- **TCB**: The TCB is a data structure that holds the critical information of
  each thread.
  1. The TID of the thread is the id of each thread which lets us identify the
     threads
  2. The context of the thread that contains the thread’s execution context.
  3. The stack pointer of a thread which is a void pointer to the stack of where
     the information is held
  4. The state of the thread helps us identify what is going on with the thread.
- **threadQueue**: This would generate a queue using the queue API and keep
  track of current zombie and ready threads.
- **readyQueue**: This queue would only used to keep track of current ready
  theads. When a thread creates or exits will enqueue or dequeue on this queue.

#### Functions

- **Start**

  To begin threading, `uthread_start()` is called to initialize the main thread
  by defining the TCB struct and all of its attributes. We call
  `uthread_ctx_alloc_stack()` to allocate the stack for our thread. We assign
  the void pointer value to our `TCB->stackp`. Lastly,
  `uthread_ctx_init(context, stackp,NULL)` is used to initialize the thread's
  execution.

- **Stop**

  When there is no more threads ready in the queue meaning there are no more
  runnable threads, the program will free the malloced spaces and reutrn 0.
  Ifthere is still runnable threads, and the program calls stop, it will return
  withvalue -1.

- **Create**

  `uthread_create()` instantiate a new TCB and assigns all the neccessary values
  to the **TID**, **state**, and **stack**. `uthread_ctv_init()` is called to
  start the thread's function. The threadQueue and readyQueue will also enqueue
  thread, since a thread is always ready when created.

- **Yield**

  `uthread_yield()` is called when the timer is up or when a thread voluntarily
  gives up its resources. The first thread in the queue is removed and if it is
  still in a running state, we enqueue it at the end.

  We want to make sure the nextThread that we switch to is ready, if the state
  is not ready, which could be in zombie or blocked statge, the function would
  call dequeue and enqueue again to suffle up the next ready thread.

  In the case when the previousThread is equal to currentThread, we don't need
  to context switch, therefore we simply remove it from the readyQueue so that
  the join function can returns and execute the function.

  We call code below to switch between two threads

  `uthread_ctx_switch(previousThread->context, currentThread->context);`

- **Join**

  thread_join(TID, retval). Similar to waitpid(), the goal of this phase is to
  stop an active thread until the child thread dies or terminates. The steps of
  the implementation are as follows.

  1. Identify the thread in the queue with the TID given.
  2. Check if the state of the child process is **terminated**. If so, free the
     space and collect the return value.
  3. If the child thread is still active, block the calling thread yield without
     context switching.

  Function flow: The parent thread when enters uthread_join would first be
  blocked and removed from the threadQueue. It would then look for the
  childThread in the queue using the iterate function from queue API with a
  special static callback function which allows the iterate function to stop
  when reaches child TID.

  If the child fucntion is still in ready state, then this function would
  trigger yield fucntion until the child exits and is in zombie state.

  If the child funtion is in zombie state, that means that the parent can remove
  the child and free its resouces. Such child would also be deleted from the
  threadQueue.

  Then the parent thread would be unblocked and turned into ready state. This
  parent thread would be enqueued to the threadQueue and all the runnable
  threads in the threadQueue would be run.

  We can know there is still runnable threads in the threadQueue by inspecting
  the queue_length of readyQueue since readyQueue would only contains runnable
  threads.

## Preempt

### Preempt_Start

Phase 4 is the implementation of preempt in our thread switch. We set up a timer
by using the struct of itimerval and a sigaction to initialize our signal
handler. The signal handler will call our timer function which calls our yield
function. This will force the threads to yield without voluntarily doing so.

To set the timer of the interval, we change the value of our
`itimerval interval` variable. The itimerval has two types of specification,
**it_value** and **it_interval**.

- it_value specifies the period between now and the first timer interrupt
- it_interval specifies when a timer should expire

We set the timer with:

`setitimer(ITIMER_VIRTUAL, &interval, NULL)`

### {enable, disable}

The enable, disable functions are used to ignore the timer signal during
critical areas. We create a `sigset_t signal`. We first set the signal to
exclude all other signals. Then we can add a signal to the signal set, signal.
We can block the signal by calling the
`sigprocmask(int how, const sigset_t *restrict set, sigset_t * restrict oldset)`
function by passing **SIG_BLOCK** into how of sigprocmask.

Enable and disable code are the same but uses SIG_UNBLOCK and SIG_BLOCK
respectively in the sigprocmask function.

### code

```
sigset_t signal;
sigemptyset(&signal);
sigaddset(&signal, SIGVTALRM); //SIGVALRM is an alarm signal for a virtual time alarm
sigprocmask(SIG_BLOCK,&signal, NULL);
```

### Testing

In order to test our preempt code, we created test*preempt.c to determine
whether the threads will switch without calling on the yield function. I created
3 thread functions that all have a while loop. In the while loop, we constantly
print “Thread* is running”. We run this in every thread to see whether the
threads are forced to yield when a certain amount of time is up.
