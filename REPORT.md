# Uthread Library: A Basic User-level Thread Library for Linux

## Summary

This program, 'Uthread Library', is a user-level thread library that provides a
complete interface for applications to create and run independents threads 
concurrently.

## Implementation

The implementation of this program follows four distinct steps:
1. Construct the underlying data structure queue to store threads. 
2. Create struct TCB and global variables to store information about each thread
and the thread library as the whole.
3. Construct library functions for this cooperative user-level thread library.
4. Add preemption function to this library.

Note: Variables names are **bold**. Function names are _italic_.

### The Underlying Data Structure Queue

#### Composition of Queue
Our queue is implemented as a linked list, and has three members **head**, 
**tail**, and **size**. **head** and **tail** point to the first and the last
node in the queue, and the **size** keeps track of the length of the queue.
We also created a struct for each node in the queue, which has members **value** 
points to its own value and **next** points to the next node in the queue.

#### Implemented Functions for Queue
Functions _queue\_enqueue_ and _queue\_dequeue_ work properly by changing what 
the first node and the last node point to, and what **head** and **tail** point
to. Function _queue\_delete works properly by having two nodes **prev** and 
**curr**. Once **curr** reachs to node needs to be deleted, **prev** will point
to the node **curr** point to. Function _queue\_iterate_ can be resistant to 
deleted data items during iteration by assigning the variable **next** with next
node before calling _func_ and increamenting by assigning **curr** to **next** 
after calling _func_.

 
#### How Iterate Functions Were Used

##### Find the Next Ready Thread in the Queue
The iterate function _find\_ready_ serves for this propose. For each thread in 
the queue, if its state is ready, the iteration will stop and store this thread.
Otherwise, _queue\_dequeue_ and _queue\_enqueue_ will be called in order to move
the current first thread to the last.

##### Find the Thread Based on the Given TID
The iterate function _find\_TCB_ serves for this propose. For each thread in the
queue, if its TID is the given one, stop the iteration. Otherwise, keep
iterating.

##### Unblocking the Thread When its Joined Thread's Return Value is Collected
The iterate function _change\_join_ serves for this propse. For each thread in 
the queue, if its TID is the given one, _queue\_enqueue_ and _queue\_delete_ 
will be called in order to move the current thread to the last and stop the 
iteration. Otherwise, keep iterating.


### Struct TCB and Global Variables

#### Struct TCB
We create a struct named TCB for each thread. It contains information about one
thread's state, context, TID, stack pointer, return value(**retval**), its 
joined thread's return value(**joinretval**), the TID of the thread it's 
joined(**joinwhich**), and the TID of the thread joined itself(**joinby**).

#### Global Variables
We have two queues named **threadqueue** and **zombiequeue**. While the prior
one stores all the ready, running, and blocked threads, the later one stores 
all the threads that are exited and waiting for another thread to collect its 
return value. We have an array **ctx** to store the context of each suspended 
threads. We have two TCBs. One for the main thread, and one for the current 
thread. We have **AllTID** to keep track total number of threads created. We 
have an array **activethreads**, and **activethreads[i]** == 1 indicates the
thread with TID of i either in the **threadqueue** or **zombiequeue**.

### Construct Library Functions

#### Initialization of the Thread Library 
The thread library is initialized by creating **threadqueue**, **zombiequeue**
and assigning the current thread to two global variables main thread and the 
current thread. Also, the main thread will be enqueued into **threadqueue**.

#### Four States A Thread Can Be
This part will be explained based on four possible states a thread can be. 
Namely running, ready, blocked, and zombie, and how a thread can transfer 
between each state. 

##### When a Thread is Running
A running thread will be the first thread in **threadqueue**. If it joins to
another thread, if that thread is in **threadqueue**, its state will be changed
to blocked, and the function _queue\_iterate_ will be called with iterate 
function _find\_ready_ as explained above to find the next ready thread in 
**threadqueue**. **retval** will be assigned with this thread's **joinretval** 
if it's not NULL. If that thread is in **zombiequeue**, **retval** will be 
assigned with that thread's **retval** if it's not NULL. Also the current thread
will be remained in running state. If it calls _uthread\_yield_, 
_queue\_dequeue_ and _queue\_enqueue_ will be called in order to move this 
thread to the last, and _find\_ready_ will again be used on **threadqueue**. If 
this thread finishs all of its code, _uthread\_exit_ will be called. If this 
thread's **joinby** is not 0 or its joined by main thread, which indicates it's 
currently being joined by another thread, then _queue\_iterate_ will be called 
with iterate function _find\_TCB_ by passing this thread's **joinby** as one of
arguments to find matched TID. Then the found thread's **joinretval** will be 
assigned with **retval**. If this thread has not been joined, it will be 
enqueued into **zombiethread**. For all conditions, once a thread's state 
changed from running to something else, it will be dequeued out of 
**threadqueue**. (It can be enqueued right after, but dequeued first)

##### When a Thread is Ready
A ready thread will be a thread in **threadqueue**. It changes its state to 
running when it becomes the first thread in **threadqueue**. 

##### When a Thread is Blocked
A blocked thread will be a thread in **threadqueue**, and it's currently joining
another thread. Once that thread finishes, _queue\_iterate_ will be called with 
iterate function _change\_join_ to move this blocked thread to the end of 
**threadqueue** and change the state to ready. 

##### When a Thread is Zombie
A zombie thread is in **zombiethread**, and it waits for a thread to collect its
return value. Once its collected, free(this thread) can be called. 

### Preemption Implementation

#### An Overall Implementation
We have create two global struct itimerval **timer**, **timer_restore** and two
global struct sigaction **signal_handler** and **signal_handler_restore**, and 
a global sigset\_t **signal_sets**. **timer** is initialized with 10hz. 
**signal_sets** is initialized with handling SIGVTALRM only. **signal_handler**
is initialized with **signal_sets** and a function leads to _uthread\_yield_. 
Then we call _sigaction_ and _setitimer_ with aboe arguments can successfully 
set the alarm and receive signals. In stop function, 10hz will be replaced with
0 to disable the alarm. Timer and signal handler will be restored by assigning
to **timer_restore** and **signal_handler_restore**. Disable and enable work by
using function _sigprocmask_ with the first argument SIG\_UNBLOCK and SIG\_BLOCK.

#### Critical Sections for Enable and Disable
Since threads share heap and data, all instructions involved changed of global
variables or malloced spaces would be considered as critical sections. Thus,
arounded by _preempt\_disable_ and _preempt\_enable_.

#### How Testers Demonstrates Our Preemptive Scheduler Works
We have four tester files for phase 4. They all contain three threads with
thread 1 creates therad 2, thread 2 creates thread 3, and there is an infinite 
loop in thread 2. In test\_preempt.c, we test for _preempt\_start_ by passing 
**preempt** == 1 to _uthread\_start_. And the output is thread 1 thread 3 with 
no thread 2, thus works. Then, in test\_preempt\_stop.c, we test for 
_preempt\_stop_ by calling _preempt\_stop_ after _uthread\_start_. It turns out
after print thread 1, the program stuck into infinite loop, thus works. Then, in
in test\_preempt\_disable.c, we test for _preempt\_disable_ by calling 
_preempt\_disable_ before the infinite loop, and the result turns out after 
print thread 1, the program stuck into infinite loop, thus works. Finally, in
test\_preempt\_enable.c, we test for _preempt\_enable_ by calling 
_preempt\_disable_ and _preempt\_enable_ before the infinite loop, and the 
output is thread 1 with thread 3, thus works.

## Extensibility

Note: Macro names are **bold**.

### Macro
Maximum number of threads can be created is defined in macro as **USHRT_MAX**. 
The state of a thread can be **ready**, **running**, or **blocked**, and is 
defined as 0, 1, and 2 in macro correspondingly. If a thread is not in 
**threadqueue** and **zombiequeue**, then this thread is **inactive**, has 
defined with value 0 in macro, and 1 with **active**.

## Limitations 
1. We can not handle the situation when thread 1 joins thread 2, thread 2 joins
thread 3, and thread 3 joins thread 1. 
2. In our program, if the user calls _uthread\_start_ after calling 
_uthread\_stop_, then our tid will not restart from 1. The main is still 0.
3. Once a thread is being freed, its TID will not be reused. For example, if
there are currently five threads besides the main thread, and the thread two
is being freed, then the new thread will take TID of 6 not 2.













 




