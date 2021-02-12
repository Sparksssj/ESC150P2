#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "queue.h"

/* Callback function that increments integer items by a certain value (or delete
 * item if item is value 42) */
static int inc_item(queue_t q, void *data, void *arg)
{
    int *a = (int*)data;
    int inc = (int)(long)arg;

    if (*a == 42)
        queue_delete(q, data);
    else
        *a += inc;

    return 0;
}

/* Callback function that finds a certain item according to its value */
static int find_item(queue_t q, void *data, void *arg)
{
    int *a = (int*)data;
    int match = (int)(long)arg;
    (void)q; //unused

    if (*a == match)
        return 1;

    return 0;
}

// Set global variables for testers.
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

/* iterator */
void test_iterator(void)
{
    queue_t q;
    int data[] = {1, 2, 3, 4, 5, 42, 6, 7, 8, 9};
    size_t i;
    int *ptr;

    /* Initialize the queue and enqueue items */
    q = queue_create();
    for (i = 0; i < sizeof(data) / sizeof(data[0]); i++)
        queue_enqueue(q, &data[i]);

    fprintf(stderr, "*** TEST queue_iterate_func_return1 ***\n");

    /* Add value '1' to every item of the queue, delete item '42' */
    queue_iterate(q, inc_item, (void*)1, NULL);
    TEST_ASSERT(data[0] == 2);
    TEST_ASSERT(queue_length(q) == 9);

    fprintf(stderr, "*** TEST queue_iterate_data_notNULL ***\n");

    /* Find and get the item which is equal to value '5' */
    ptr = NULL;     // result pointer *must* be reset first
    queue_iterate(q, find_item, (void*)5, (void**)&ptr);
    TEST_ASSERT(ptr != NULL);
    TEST_ASSERT(*ptr == 5);
    TEST_ASSERT(ptr == &data[3]);
}

void test_queue_complex1(void)
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


}

void test_queue_complex2(){

    // Case 2
    q = queue_create();
    queue_enqueue(q, &data);
    queue_enqueue(q, &newdata);
    queue_dequeue(q, (void**)&ptr);
    queue_enqueue(q, ptr);
    queue_dequeue(q, (void**)&ptr);
    TEST_ASSERT(ptr == &newdata && queue_length(q) == 1 );
    queue_dequeue(q, (void**)&ptr);
    TEST_ASSERT(!queue_destroy(q));
}

void test_queue_complex3() {
    queue_t q;
    int data[] = {1, 2, 3, 4, 5, 42, 6, 7, 8, 9};
    size_t i;
    int *ptr;

    q = queue_create();
    for (i = 0; i < sizeof(data) / sizeof(data[0]); i++)
        queue_enqueue(q, &data[i]);

    queue_delete(q, &data[2]);
    TEST_ASSERT(queue_length(q) == 9);
    queue_dequeue(q, (void**)&ptr);
    queue_dequeue(q, (void**)&ptr);
    queue_dequeue(q, (void**)&ptr);
    TEST_ASSERT(ptr == &data[3]);
}


void test_queue_complex4() {
    queue_t q;
    int data[] = {1, 2, 3, 4, 5, 42, 6, 7, 8, 9};
    size_t i;
    int *ptr;

    q = queue_create();
    for (i = 0; i < sizeof(data) / sizeof(data[0]); i++)
        queue_enqueue(q, &data[i]);

    queue_delete(q, &data[0]);
    queue_dequeue(q, (void**)&ptr);
    TEST_ASSERT(ptr == &data[1]);
}

void test_queue_complex5() {
    queue_t q;
    int data[] = {1, 2, 3, 4, 5, 42, 6, 7, 8, 9};
    size_t i;
    int *ptr;

    q = queue_create();
    for (i = 0; i < sizeof(data) / sizeof(data[0]); i++)
        queue_enqueue(q, &data[i]);

    queue_delete(q, &data[9]);
    for (i = 1; i < sizeof(data) / sizeof(data[0]); i++)
        queue_dequeue(q, (void**)&ptr);
    TEST_ASSERT(ptr == &data[8]);
}

void test_queue_complex6() {
    queue_t q;
    int data[] = {1, 2, 3, 4, 5, 42, 6, 7, 8, 9};
    size_t i;
    int *ptr;

    q = queue_create();
    for (i = 0; i < sizeof(data) / sizeof(data[0]); i++)
        queue_enqueue(q, &data[i]);

    queue_delete(q, &data[9]);
    TEST_ASSERT(queue_length(q) == 9);

    queue_dequeue(q, (void**)&ptr);
    queue_dequeue(q, (void**)&ptr);
    TEST_ASSERT(queue_length(q) == 7);
    queue_delete(q, &data[7]);
    TEST_ASSERT(queue_length(q) == 6);
}

int main(void)
{
    // Test basic functions with all individual scenarios.
	test_create();
	test_queue_destory();
    test_queue_enqueue();
    test_queue_dequeue();
    test_queue_delete();
    test_queue_length();
    test_iterator();
    // Test complex situations that composed with multiple functions.
    test_queue_complex1();
    test_queue_complex2();
    test_queue_complex3();
    test_queue_complex4();
    test_queue_complex5();
    test_queue_complex6();

    return 0;
}
