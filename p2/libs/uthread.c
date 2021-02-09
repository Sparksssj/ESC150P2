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
#include "queue.c"


typedef struct TCB* TCB_t;
typedef struct ZombieState* Zombie_t;


struct ZombieState{
    uthread_t TID;
    int retval;
};

struct TCB{
    uthread_t TID;
    uthread_ctx_t *context;
    void* stackptr;
    int state;
    int retval;
    int joinretval;
    uthread_t joinwhich;
    uthread_t joinby;
};

//Globa variables
queue_t threadqueue;
uthread_t AllTID = 0;
queue_t zombiequeue;
TCB_t Currentthread;
TCB_t Mainthread;
//queue_t blockedqueue;
ucontext_t ctx[USHRT_MAX];
int activethreads[USHRT_MAX];

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

Zombie_t Zombie_init(uthread_t TID, int retval){
    Zombie_t Zombie = malloc(sizeof *Zombie);
    Zombie->TID = TID;
    Zombie->retval = retval;
    return Zombie;
}

static int change_join(queue_t q, void *data, void *arg)
{
    TCB_t TCB = data;
    uthread_t TID = (uthread_t)(long) arg;

    if (TCB->TID == TID){
        TCB->state = 0;
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

int uthread_start(int preempt)
{
    if (preempt){
        return 0;
    }
    // Not called by the main thread
    if (AllTID){
        return -1;
    }

    threadqueue = queue_create();
    zombiequeue = queue_create();
    //blockedqueue = queue_create();

    void* mainstackptr = uthread_ctx_alloc_stack();

    ucontext_t* mainctx = NULL;

    //uthread_ctx_init(mainctx, mainstackptr, NULL);

    TCB_t MainTCB = TCB_init(AllTID, mainctx, mainstackptr, 0, 0, 0 , 0, 0);
    //queue_enqueue(threadqueue, MainTCB);
    Mainthread = MainTCB;
    Currentthread = MainTCB;

	return 0;
}



int uthread_stop(void)
{
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
    AllTID++;
    uthread_t ThisTID = AllTID;
    ucontext_t thisctx = ctx[ThisTID];

    void* thisstackptr = uthread_ctx_alloc_stack();

    // failure in context creation
    if(uthread_ctx_init(&thisctx, thisstackptr, func)){
        return -1;
    }

    TCB_t ThisTCB = TCB_init(ThisTID, &thisctx, thisstackptr, 0, 0, 0, 0, 0);

    // failure in memory allocation
    if (!ThisTCB){
        return -1;
    }
    ctx[ThisTID] = (*ThisTCB->context);
    queue_enqueue(threadqueue, ThisTCB);
    activethreads[ThisTID] = 1;

    return ThisTID;
}

void uthread_yield(void)
{
    TCB_t thisthread;
    queue_dequeue(threadqueue, (void **) &Currentthread);
    queue_enqueue(threadqueue, Currentthread);
    thisthread = Currentthread;

    while (((TCB_t)threadqueue->head->value)->state != 0) {
        TCB_t movetoback;
        queue_dequeue(threadqueue, (void **)&movetoback);
        queue_enqueue(threadqueue, movetoback);
    }

    Currentthread = threadqueue->head->value;
    thisthread->state = 0;
    Currentthread->state = 1;
    uthread_ctx_switch(&ctx[thisthread->TID], &ctx[Currentthread->TID]);

}

uthread_t uthread_self(void)
{
    if (Currentthread != Mainthread){
        return ((TCB_t)threadqueue->head->value)->TID;
    }
	return 0;
}

void uthread_exit(int retval)
{
    TCB_t thisthread;
    queue_dequeue(threadqueue, (void **) &thisthread);
    uthread_t thisTID = thisthread->TID;
    // exit thread is joined
    if ((thisthread->joinby) || (Mainthread->joinwhich == thisTID)){
        if (!queue_length(threadqueue)){
            Mainthread->joinretval = retval;
            free(thisthread);
            Currentthread = Mainthread;
            uthread_ctx_switch(&ctx[thisTID], &ctx[0]);
            return;
        }
        if (Mainthread->joinwhich == thisTID){
            Mainthread->joinretval = retval;
            Currentthread = Mainthread;
            uthread_ctx_switch(&ctx[thisTID], &ctx[0]);
            return;
        }
        queue_iterate(threadqueue, change_join, (void*)(long)thisthread->joinby, NULL);
        ((TCB_t )threadqueue->tail->value)->joinretval = retval;
        activethreads[thisTID] = 0;
        free(thisthread);
    } else {
        queue_enqueue(zombiequeue, thisthread);
        thisthread->retval = retval;
        if (!queue_length(threadqueue)){
            Currentthread = Mainthread;
            uthread_ctx_switch(&ctx[thisTID], &ctx[0]);
            return;
        }
    }
    while (((TCB_t)threadqueue->head->value)->state != 0) {
        TCB_t movetoback;
        queue_dequeue(threadqueue, (void **)&movetoback);
        queue_enqueue(threadqueue, movetoback);
    }
    Currentthread = threadqueue->head->value;
    Currentthread->state = 1;
    uthread_ctx_switch(&ctx[thisTID], &ctx[Currentthread->TID]);
}

int uthread_join(uthread_t tid, int *retval)
{
    TCB_t joinedthread_w = NULL;
    TCB_t joinedthread_z = NULL;
    queue_iterate(threadqueue, find_TCB, (void*)(long)tid, (void**)&joinedthread_w);
    queue_iterate(zombiequeue, find_TCB, (void*)(long)tid, (void**)&joinedthread_z);
    if (tid == 0 || tid == Currentthread->TID || !activethreads[tid] || (joinedthread_w && joinedthread_w->joinby)){
        return -1;
    } else {
        if (joinedthread_z){
            *retval = joinedthread_z->retval;
            free(joinedthread_z);
            return 0;
        } else {
            TCB_t thisthread = Currentthread;
            thisthread->state = 2;
            thisthread->joinwhich = joinedthread_w->TID;
            joinedthread_w->joinby = thisthread->TID;
            while (((TCB_t)threadqueue->head->value)->state != 0) {
                TCB_t movetoback;
                queue_dequeue(threadqueue, (void **)&movetoback);
                queue_enqueue(threadqueue, movetoback);
            }
            Currentthread = threadqueue->head->value;
            Currentthread->state = 0;
            uthread_ctx_switch(&ctx[thisthread->TID], &ctx[Currentthread->TID]);
            *retval = thisthread->joinretval;
            return 0;
        }
    }
}

