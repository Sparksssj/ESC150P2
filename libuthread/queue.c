#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "queue.h"

/*
 * Defining the queue using linked list, in which there are many nodes.
 */
typedef struct Node* Node_t;

/*
 * Each node has it's own value and the pointer to the next node.
 */
struct Node {
    void* value;
    struct Node* next;
};

/*
 * Initialize a node.
 */
Node_t node_create(void* data){
    Node_t node = malloc(sizeof *node);
    node->value = data;
    node->next = NULL;
    return node;
}

/*
 * Each queue has a head, a tail, and it's length.
 */
struct queue {
    Node_t head;
    Node_t tail;
    size_t size;
};

/*
 * Initialize a queue.
 */
queue_t queue_create(void)
{
    queue_t queue = malloc(sizeof *queue);
    queue->head = NULL;
    queue->tail = NULL;
    queue->size = 0;

	return queue;
}

int queue_destroy(queue_t queue)
{
    /* Return -1 when queue doesn't exist or the queue is not empty*/
	if ((!queue) || (queue->size)){
	    return -1;
	}
	/* Return 0 in case of succeed*/
	free(queue);
	return 0;
}

int queue_enqueue(queue_t queue, void *data)
{
    /* Return -1 when data is NULL or queue doesn't exist*/
	if ((!data) || (!queue)){
	    return -1;
	}
	/* Return -1 when failed to create a node*/
	Node_t node = node_create(data);
	if (!node){
	    return -1;
	}
	/* When queue is empty, set the head as the new node.
	 * Set the new node to the tail of the queue otherwise and return 0.
	 * */
	if (queue->size == 0){
	    queue->head = node;
        queue->tail = node;
	} else {
        queue->tail->next = node;
        queue->tail = node;
	}
	queue->size++;
	return 0;
}

int queue_dequeue(queue_t queue, void **data)
{
    /* Return -1 if queue doesn't exist or @data doesn't exist or the queue is empty*/
	if ((!queue) || (!data) || (queue->size == 0)){
        return -1;
	}

	/* Set the tail of queue to NULL in the case that there's only 1 element in the queue*/
	if (queue->size == 1){
	    queue->tail = NULL;
	}

	/* Using temporary value to change the head of queue.*/
	*data = queue->head->value;
	Node_t temp_node = queue->head;
	queue->head = queue->head->next;
	temp_node->next = NULL;
	queue->size--;

	free(temp_node);
	return 0;
}

int queue_delete(queue_t queue, void *data)
{
    /* Return -1 if queue doesn't exist or @data doesn't exist or the queue is empty*/
	if ((!queue) || (!data) || (queue->size == 0)){
        return -1;
	}


	Node_t prev = queue->head;
	Node_t find = queue->head;
	/* Delete the node and manage the pointers when find the data*/
	if (find->value == data){
        Node_t temp_node = queue->head;
        queue->head = queue->head->next;
        temp_node->next = NULL;
        queue->size--;
        free(temp_node);
	    return 0;
	}
	find = find->next;
	while (find){
	    if (find->value == data){
	        if (!find->next){
	            queue->tail = prev;
	        }
	        prev->next = find->next;
	        find->next = NULL;
	        free(find);
            queue->size--;
	        return 0;
	    }
	    find = find->next;
	    prev = prev->next;
	}
	return -1;
}

int queue_iterate(queue_t queue, queue_func_t func, void *arg, void **data)
{
    /* Return -1 when queue and function don't exist.*/
	if ((!queue) || (!func)){
        return -1;
	}
	Node_t curr = queue->head;
	int stop;
	/* Use loop to apply the function to all the nodes and collect data.*/
	while (curr){
	    Node_t next = curr->next;
	    stop = func(queue, curr->value, arg);
	    if (stop){
	        if (data){
	            *data = curr->value;
	        }
	        break;
	    }
	    curr = next;
	}
	return 0;
}

int queue_length(queue_t queue)
{
    /* Return -1 if the queue doesn't exist, return length otherwise.*/
	if (!queue) {
	    return -1;
	}
	return queue->size;
}

