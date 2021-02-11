#include <assert.h>
#include <limits.h>
#include <signal.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#include "private.h"
#include "uthread.h"
#include "queue.h"

#define READY 0
#define RUNNING 1
#define BLOCKED 2
#define ACTIVE 1
#define INACTIVE 0
typedef struct TCB* TCB_t;


/* Struct TCB stores the information of a thread.*/
struct TCB {
    // It's TID.
    uthread_t TID;
    // It's context.
    uthread_ctx_t *context;

    // It's stack pointer.
    void* stackptr;
    // It's state. (Blocked, Running, Ready)
    int state;
    // It's return value.
    int retval;
    // The return value of the thread it joined.
    int joinretval;
    // The TID of the thread it joined.
    uthread_t joinwhich;
    // The TID of it's parent thread.
    uthread_t joinby;
};

/* Global variables*/

// Two queues to be used globally
queue_t threadqueue;
queue_t zombiequeue;

// The TID number to be signed to a new thread, which is initialized with 0.
uthread_t AllTID = 0;
// A variable to trace the current thread.
TCB_t Currentthread;
// A variable to record the main thread.
TCB_t Mainthread;
// Two arrays to record the context of all threads and whether a thread is active or not.
ucontext_t ctx[USHRT_MAX];
int activethreads[USHRT_MAX];

/* Initialize TCB*/
TCB_t TCB_init(uthread_t TID, ucontext_t* context, void* stackptr, int state, int retval, int joinretval, uthread_t joinwhich, uthread_t joinby){
    TCB_t TCB = malloc(sizeof *TCB);
    TCB -> TID = TID;
    TCB -> context = context;
    TCB -> stackptr = stackptr;
    //0=ready, 1 = running, 2 = blocked
    TCB -> state = state;
    TCB -> retval = retval;
    TCB -> joinretval = joinretval;
    TCB -> joinwhich = joinwhich;
    TCB -> joinby = joinby;
    return TCB;
}

static int change_join(queue_t q, void *data, void *arg)
{
    TCB_t TCB = data;
    uthread_t TID = (uthread_t)(long) arg;

    if (TCB->TID == TID){
        TCB->state = READY;
        queue_enqueue(q, TCB);
        queue_delete(q, TCB);
        return 1;
    }
    return 0;
}

static int find_TCB(queue_t q, void *data, void *arg)
{
    TCB_t TCB = data;
    uthread_t TID = (uthread_t)(long) arg;
    (void)q; //unused

    if (TCB->TID == TID){
        return 1;
    }
    return 0;
}

static int find_ready(queue_t q, void *data, void *arg)
{
    TCB_t TCB = data;

    // unused block
    (void)q;
    int TID = (uthread_t)(long) arg;
    if (TID == -1){
        return 0;
    }

    if (TCB->state == READY){
        return 1;
    }
    queue_dequeue(q, (void**)&data);
    queue_enqueue(q, data);
    return 0;
}

/* Initialize the multithreads working.*/
int uthread_start(int preempt)
{
    // Not called by the main thread
    if (AllTID){
        return -1;
    }

    // Enable preempt if receive the argument of 1.
    if (preempt){
        preempt_start();
    }

    // Create the queues.
    threadqueue = queue_create();
    zombiequeue = queue_create();

    // Initialize the Mainthread, and store it properly.
    void* mainstackptr = uthread_ctx_alloc_stack();
    ucontext_t mainctx = ctx[0];
    TCB_t MainTCB = TCB_init(AllTID, &mainctx, mainstackptr, RUNNING, 0, 0 , 0, 0);

    // failure in memory allocation
    if (!MainTCB){
        return -1;
    }

    Mainthread = MainTCB;
    // Set current thread.
    Currentthread = MainTCB;
    // Add the main thread to the thread queue.
    queue_enqueue(threadqueue, Mainthread);
    return 0;
}



int uthread_stop(void)
{
    // Stop preemption.
    preempt_stop();
    // Only if it's called in Main function and all the queues are empty, it will stop.
    if ((Currentthread == Mainthread) && (!queue_length(threadqueue)) && (!queue_length(zombiequeue))){
        queue_destroy(threadqueue);
        queue_destroy(zombiequeue);
        return 0;
    }
    return -1;
}

int uthread_create(uthread_func_t func)
{
    // TID overflow
    if (AllTID == USHRT_MAX){
        return -1;
    }
    // Create TID for the new thread.
    AllTID++;
    uthread_t ThisTID = AllTID;
    ucontext_t thisctx = ctx[ThisTID];

    // Initialize the thread, and store it properly.
    void* thisstackptr = uthread_ctx_alloc_stack();

    // failure in context creation
    if(uthread_ctx_init(&thisctx, thisstackptr, func)){
        return -1;
    }
    TCB_t ThisTCB = TCB_init(ThisTID, &thisctx, thisstackptr, READY, 0, 0, 0, 0);

    // failure in memory allocation
    if (!ThisTCB){
        return -1;
    }
    ctx[ThisTID] = (*ThisTCB->context);

    // Add the new thread in the thread queue, and change it to active.
    queue_enqueue(threadqueue, ThisTCB);
    activethreads[ThisTID] = ACTIVE;

    return ThisTID;
}

void uthread_yield(void)
{
    // When yield, put the current thread to the end of thread queue.
    TCB_t thisthread;
    queue_dequeue(threadqueue, (void **) &Currentthread);
    queue_enqueue(threadqueue, Currentthread);
    thisthread = Currentthread;

    // If the first thread is not ready, push it to the back.
    queue_iterate(threadqueue, find_ready, thisthread, (void**)&Currentthread);

    // Change to the next available thread.
    thisthread->state = READY;
    Currentthread->state = RUNNING;
    uthread_ctx_switch(&ctx[thisthread->TID], &ctx[Currentthread->TID]);

}

uthread_t uthread_self(void)
{
    // Return the TID of current thread.
    return Currentthread->TID;
}

void uthread_exit(int retval)
{
    // Delete the exit thread from the queue.
    TCB_t thisthread;
    TCB_t unblockedthread;
    queue_dequeue(threadqueue, (void **) &thisthread);
    uthread_t thisTID = thisthread->TID;

    // If exit thread is joined.
    if ((thisthread->joinby) || (Mainthread->joinwhich == thisTID)){
        // Find the parent thread and give the return value to it.
        queue_iterate(threadqueue, change_join, (void*)(long)thisthread->joinby, NULL);
        queue_iterate(threadqueue, find_TCB, (void*)(long)thisthread->joinby, (void**)&unblockedthread);
        unblockedthread->joinretval = retval;
        // Set the thread to inactive.
        activethreads[thisTID] = INACTIVE;
        free(thisthread);
    } else {
        // Send it to zombie thread if it is not joined.
        queue_enqueue(zombiequeue, thisthread);
        thisthread->retval = retval;
    }
    // Find the next available value.
    queue_iterate(threadqueue, find_ready, thisthread, (void**)&Currentthread);

    // Change to the next available thread.
    Currentthread->state = RUNNING;
    uthread_ctx_switch(&ctx[thisTID], &ctx[Currentthread->TID]);
}

int uthread_join(uthread_t tid, int *retval)
{
    TCB_t joinedthread_w = NULL;
    TCB_t joinedthread_z = NULL;
    queue_iterate(threadqueue, find_TCB, (void*)(long)tid, (void**)&joinedthread_w);
    queue_iterate(zombiequeue, find_TCB, (void*)(long)tid, (void**)&joinedthread_z);


    /*
     * Return -1 if the thread is trying to join main thread or itself, or an exited thread,
     * or and non-exist thread, or a thread is already joined by other thread.
     * */
    if (tid == 0 || tid == Currentthread->TID || !activethreads[tid] || (joinedthread_w && joinedthread_w->joinby)){
        return -1;
    } else {
        // If the thread is already in zombie state, collect it right away if needed.
        if (joinedthread_z){
            if (retval){
                *retval = joinedthread_z->retval;
            }
            free(joinedthread_z);
            return 0;
        } else {
            // Block the current state.
            TCB_t thisthread = Currentthread;
            thisthread->state = BLOCKED;
            // Set TCB of the relationship of these two threads.
            thisthread->joinwhich = joinedthread_w->TID;
            joinedthread_w->joinby = thisthread->TID;

            // Find the next available thread, and run it.
            queue_iterate(threadqueue, find_ready, thisthread, (void**)&Currentthread);
            Currentthread->state = RUNNING;
            uthread_ctx_switch(&ctx[thisthread->TID], &ctx[Currentthread->TID]);

            // Collect return value if needed.
            if (retval){
                *retval = thisthread->joinretval;
            }
            return 0;
        }
    }
}