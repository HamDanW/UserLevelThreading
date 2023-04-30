#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "queue.h"

struct queue {
    void** arr;
    size_t size;
    size_t oldest_pos;
};

queue_t queue_create(void)
{
    queue_t q = (queue_t) malloc (sizeof(struct queue ));
    q->arr = (void**) malloc (sizeof(void*));
    q->size = 0;
    q->oldest_pos =0;
    return q;
}
int queue_destroy(queue_t queue)
{   
	if(queue == NULL || queue->arr == NULL || queue_length(queue) != 0){
		return -1;
	}
	free(queue->arr);
	return 0;
}

int queue_enqueue(queue_t queue, void *data)
{
    if(queue == NULL || queue->arr == NULL || data == NULL)
        return -1;
    queue->arr = (void**)realloc(queue->arr, (queue->size+1) * sizeof(void*));
    if(queue->arr == NULL)
        return -1;
    queue->arr[queue->size] = data;
    queue->size = queue->size + 1;
    return 0;
}

int queue_dequeue(queue_t queue, void **data)
{
    if(queue == NULL || queue->arr == NULL || data == NULL || queue->size == 0){
        return -1;
    }
    *data =  queue->arr[queue->oldest_pos];
    queue->oldest_pos = queue->oldest_pos + 1;
    return 0;
}

int queue_delete(queue_t queue, void *data)
{
    if(queue == NULL ||queue->arr == NULL || data == NULL ){
        return -1;
    }
    for (size_t i = queue->oldest_pos; i < queue->size; i++){
        if(queue->arr[i] == data){
            for (size_t j = i; j < queue->size -1; j++){
                queue->arr[j] = queue->arr[j+1];
            }
            queue->arr[queue->size-1] = NULL;
            queue->size = queue->size -1;
            return 0;
        }
    }
    return -1;
}

int queue_iterate(queue_t queue, queue_func_t func, void *arg, void **data)
{
    if(queue == NULL ||queue->arr == NULL || func == NULL ){
        return -1;
    }
    for(size_t i =queue->oldest_pos; i < queue->size; i++){
        void * cur_data  = queue->arr[i];
        int func_return = func(queue, cur_data, arg);
        if(func_return == 1){
            if(*data != NULL){
                *data = cur_data;
            }
            return 0;
        }

    }
    return 0;
}
int queue_length(queue_t queue)
{   
    if(queue == NULL || queue->arr == NULL)
        return -1;
    return (int)(queue->size - queue->oldest_pos);
}
