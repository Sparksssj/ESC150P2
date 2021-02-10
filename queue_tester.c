#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "queue.h"

// Set global variables.
int data = 3, *ptr, newdata = 4;
queue_t q;

#define TEST_ASSERT(assert)				\
do {									\
	printf("ASSERT: " #assert " ... ");	\
	if (assert) {						\
		printf("PASS\n");				\
	} else	{							\
		printf("FAIL\n");				\
		exit(1);						\
	}									\
} while(0)

/* Create */

void test_create(void)
{
	fprintf(stderr, "*** TEST create ***\n");

	TEST_ASSERT(queue_create() != NULL);
}



void test_queue_destory(void){
    fprintf(stderr, "*** TEST queue_destroy***\n");

    // queue_destroytests
    q = queue_create();
    int destroytest1, destroytest2, destroytest3, destroytest4;

    // Simply destroy a empty queue.
    destroytest1 = queue_destroy(q);
    TEST_ASSERT(destroytest1 == 0);

    // Destroy a queue when there are elements in it.
    q = queue_create();
    queue_enqueue(q, &data);
    destroytest2 = queue_destroy(q);
    TEST_ASSERT(destroytest2 == -1);

    // Destroy a non_exist queue.
    destroytest3 = queue_destroy(NULL);
    TEST_ASSERT(destroytest3 == -1);

    // Destroy the queue for this test.
    queue_dequeue(q, (void**)&ptr);
    destroytest4 = queue_destroy(q);
    TEST_ASSERT(destroytest4 == 0);

}

void test_queue_enqueue(void){

    //queue_enqueue tests
    fprintf(stderr, "*** TEST queue_enqueue***\n");

    int enqueuetest1,enqueuetest2,enqueuetest3,enqueuetest4;

    // Simply enqueue an element.
    q = queue_create();

    enqueuetest1 = queue_enqueue(q, &data);
    TEST_ASSERT(enqueuetest1 == 0);

    // Enqueue an element to a non_exist queue.
    enqueuetest2 = queue_enqueue(NULL, &data);
    TEST_ASSERT(enqueuetest2 == -1);

    // Enqueue an NULL in a queue.
    enqueuetest3 = queue_enqueue(q, NULL);
    TEST_ASSERT(enqueuetest3 == -1);

    // Destroy the queue for this test

    queue_dequeue(q, (void**)&ptr);
    enqueuetest4 = queue_destroy(q);
    TEST_ASSERT(enqueuetest4 == 0);

}

void test_queue_dequeue(void){

    fprintf(stderr, "*** TEST queue_dequeue***\n");

    int dequeuetest1,dequeuetest2,dequeuetest3,dequeuetest4,dequeuetest5;

    q = queue_create();

    // Dequeue when queue is empty.
    dequeuetest1 = queue_dequeue(q, (void**)&ptr);
    TEST_ASSERT(dequeuetest1 == -1);

    // Dequeue when queue is not empty.
    queue_enqueue(q, &data);
    dequeuetest2 = queue_dequeue(q, (void**)&ptr);
    TEST_ASSERT(dequeuetest2 == 0 && ptr == &data);

    // Dequeue when queue is NULL
    dequeuetest3 = queue_dequeue(NULL, (void**)&ptr);
    TEST_ASSERT(dequeuetest3 == -1);

    // Dequeue when @data is NULL
    queue_enqueue(q, &data);
    dequeuetest4 = queue_dequeue(q, NULL);
    TEST_ASSERT( dequeuetest4 == -1);

    // Destroy the queue for this test.
    queue_dequeue(q, (void**)&ptr);
    dequeuetest5 = queue_destroy(q);
    TEST_ASSERT(dequeuetest5 == 0);
}

void test_queue_delete(void){
    fprintf(stderr, "*** TEST queue_delete***\n");

    int deletetest1,deletetest2,deletetest3,deletetest4,deletetest5, deletetest6;

    // Delete when queue is NULL
    deletetest1 = queue_delete(NULL, &data);
    TEST_ASSERT(deletetest1 == -1);

    // Delete when @data is NULL
    q = queue_create();
    queue_enqueue(q, &data);
    deletetest2 = queue_delete(q, NULL);
    TEST_ASSERT(deletetest2 == -1);

    // Delete when there is such data in queue.
    deletetest3 = queue_delete(q, &data);
    TEST_ASSERT(deletetest3 == 0);

    // Delete when there isn't such data in queue and queue is empty.
    int randomdata = 1;
    deletetest4 = queue_delete(q, &randomdata);
    TEST_ASSERT(deletetest4 == -1);

    // Delete when there isn't such data in queue and queue is not empty.
    queue_enqueue(q, &data);
    deletetest5 = queue_delete(q, &randomdata);
    TEST_ASSERT(deletetest5 == -1);

    // Destroy the queue for this test.
    queue_dequeue(q, (void**)&ptr);
    deletetest6 = queue_destroy(q);
    TEST_ASSERT(deletetest6 == 0);
}

void test_queue_length(void){
    fprintf(stderr, "*** TEST queue_length***\n");

    int queuelengthtest1,queuelengthtest2,queuelengthtest3,queuelengthtest4;

    // When queue is empty
    q = queue_create();
    queuelengthtest1 = queue_length(q);
    TEST_ASSERT( queuelengthtest1 == 0);

    // When queue has some elements.
    queue_enqueue(q, &data);
    queue_enqueue(q, &newdata);
    queuelengthtest2 = queue_length(q);
    TEST_ASSERT(queuelengthtest2 == 2);

    // When queue is NULL
    queuelengthtest3 = queue_length(NULL);
    TEST_ASSERT(queuelengthtest3 == -1);

    // Destroy queue for this test.
    queue_dequeue(q, (void**)&ptr);
    queue_dequeue(q, (void**)&ptr);
    queuelengthtest4 = queue_destroy(q);
    TEST_ASSERT(queuelengthtest4 == 0);
}

void test_queue_complex(void)
{

    // More complex situations tests
    fprintf(stderr, "*** Complex situation tests***\n");

    // Case 1
    q = queue_create();
    queue_enqueue(q, &data);
    queue_enqueue(q, &newdata);
    queue_delete(q, &data);
    queue_dequeue(q, (void**)&ptr);
    TEST_ASSERT(ptr == &newdata && queue_length(q) == 0 && !queue_destroy(q));

    // Case 2
    q = queue_create();
    queue_enqueue(q, &data);
    queue_enqueue(q, &newdata);
    queue_dequeue(q, (void**)&ptr);
    queue_enqueue(q, ptr);
    queue_dequeue(q, (void**)&ptr);
    TEST_ASSERT(ptr == &newdata && queue_length(q) == 1);




}
int main(void)
{
	test_create();
	test_queue_destory();
    test_queue_enqueue();
    test_queue_dequeue();
    test_queue_delete();
    test_queue_length();
    test_queue_complex();



    return 0;
}
