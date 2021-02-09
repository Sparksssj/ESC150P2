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
    int joinwhich;
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
int whetherjoined[USHRT_MAX];




TCB_t TCB_init(uthread_t TID, ucontext_t* context, void* stackptr, int state, int joinwhich){
    TCB_t TCB = malloc(sizeof *TCB);
    TCB -> TID = TID;
    TCB -> context = context;
    TCB -> stackptr = stackptr;
    TCB -> state = state;
    //0=ready, 1 = running, 2 = blocked
    TCB -> joinwhich = joinwhich;
    return TCB;
}

Zombie_t Zombie_init(uthread_t TID, int retval){
    Zombie_t Zombie = malloc(sizeof *Zombie);
    Zombie->TID = TID;
    Zombie->retval = retval;
    return Zombie;
}


int uthread_start(int preempt)
{
    threadqueue = queue_create();
    zombiequeue = queue_create();
    //blockedqueue = queue_create();

    void* mainstackptr = uthread_ctx_alloc_stack();

    ucontext_t* mainctx = NULL;

    //uthread_ctx_init(mainctx, mainstackptr, NULL);

    TCB_t MainTCB = TCB_init(AllTID, mainctx, mainstackptr, 0 , 0);
    //queue_enqueue(threadqueue, MainTCB);
    Mainthread = MainTCB;
    Currentthread = MainTCB;

	return -1;
}



int uthread_stop(void)
{
	return queue_destroy(threadqueue) == 0;
}

int uthread_create(uthread_func_t func)
{
    AllTID++;
    uthread_t ThisTID = AllTID;
    ucontext_t thisctx = ctx[ThisTID];

    void* thisstackptr = uthread_ctx_alloc_stack();

    uthread_ctx_init(&thisctx, thisstackptr, func);

    TCB_t ThisTCB = TCB_init(ThisTID, &thisctx, thisstackptr, 0, 0);
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

    while (1) {
        // When next thread is ready, just run it
        if (((TCB_t)threadqueue->head->value)->state == 0){
            //Currentthread = threadqueue->head->value;
            break;
        }
        // Else we will push the next thread to the end of the queue, and check next of the next thread.
        else {
            TCB_t movetoback;
            queue_dequeue(threadqueue, (void **)&movetoback);
            queue_enqueue(threadqueue, movetoback);
        }
    }

    Currentthread = threadqueue->head->value;
    thisthread->state = 0;
    Currentthread->state = 1;
    uthread_ctx_switch(&ctx[thisthread->TID], &ctx[Currentthread->TID]);

}

uthread_t uthread_self(void)
{
	return ((TCB_t)threadqueue->head->value)->TID;
}

void uthread_exit(int retval)
{
    Zombie_t newZombie = Zombie_init(Currentthread->TID, retval);
    activethreads[Currentthread->TID] = 0;
    queue_enqueue(zombiequeue, newZombie);
    queue_dequeue(threadqueue, (void **)&Currentthread);

    TCB_t thisthread;
    thisthread = Currentthread;

    if (threadqueue->size != 0){

        while (1) {
            /* When this first thread in queue is not the parent thread of the exit thread,
             * and this thread is blocked, it means this thread need to wait longer until the thread
             * it joined to finish. So just push this to the end of the queue.
             * */
            if ((((TCB_t)threadqueue->head->value)->joinwhich != thisthread->TID) &&  ((TCB_t)threadqueue->head->value)->state == 2 ){
                TCB_t moveback;
                queue_dequeue(threadqueue, (void **)&moveback);
                queue_enqueue(threadqueue, moveback);
            }
            // If the first thread in queue is ready, then just run it.
            else if (((TCB_t)threadqueue->head->value)->state == 0) {

                Currentthread = threadqueue->head->value;
                uthread_ctx_switch(&ctx[thisthread->TID], &ctx[Currentthread->TID]);
                break;
            }
            /* The other situation should be the first thread in queue is the parent of the exit thread.
             * In this case, we should reset the joined thread to 0, which implies it doesn't joining any thread
             * currently, and then set the state of this thread to ready.
             * */
            else {
                ((TCB_t)threadqueue->head->value)->joinwhich = 0;
                ((TCB_t)threadqueue->head->value)->state = 0;
                TCB_t moveback;
                queue_dequeue(threadqueue, (void **)&moveback);
                queue_enqueue(threadqueue, moveback);

            }
        }

    } else{
        Currentthread = Mainthread;
        uthread_ctx_switch(&ctx[thisthread->TID], &ctx[0]);
    }


}

int uthread_join(uthread_t tid, int *retval)
{
    if (tid == 0 || tid == Currentthread->TID || !activethreads[tid] || whetherjoined[tid]){
        return -1;
    } else {
        whetherjoined[tid] = 1;
        Currentthread->state = 2;
        Currentthread->joinwhich = tid;
        //queue_enqueue(blockedqueue, Currentthread);
        TCB_t thisthread = Currentthread;


        while (1) {
            if (((TCB_t)threadqueue->head->value)->state == 0){
                Currentthread = threadqueue->head->value;
                break;
            } else {
                TCB_t movetoback;
                queue_dequeue(threadqueue, (void **)&movetoback);
                queue_enqueue(threadqueue, movetoback);
            }
        }

        uthread_ctx_switch(&ctx[thisthread->TID], &ctx[Currentthread->TID]);
        return 0;
    }
}

