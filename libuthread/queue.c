#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "queue.h"

typedef struct Node* Node_t;

struct Node {
    void* value;
    struct Node* next;
};

Node_t node_create(void* data){
    Node_t node = malloc(sizeof *node);
    node->value = data;
    node->next = NULL;
    return node;
}

struct queue{
    Node_t head;
    Node_t tail;
    size_t size;
};

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
	if ((!queue) || (queue->size)){
	    return -1;
	}
	free(queue);
	return 0;
}

int queue_enqueue(queue_t queue, void *data)
{
	if ((!data) || (!queue)){
	    return -1;
	}
	Node_t node = node_create(data);
	if (!node){
	    return -1;
	}
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
	if ((!queue) || (!data) || (queue->size == 0)){
        return -1;
	}
	if (queue->size == 1){
	    queue->tail = NULL;
	}
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
	if ((!queue) || (!data)){
        return -1;
	}
	Node_t prev = queue->head;
	Node_t find = queue->head;
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
	if ((!queue) || (!func)){
        return -1;
	}
	Node_t curr = queue->head;
	int stop;
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
	if (!queue) {
	    return -1;
	}
	return queue->size;
}

