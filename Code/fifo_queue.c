// C program for array implementation of fifo queue
//reference : https://www.geeksforgeeks.org/queue-set-1introduction-and-array-implementation/
#include "fifo_queue.h"

 
// function to create a queue of given capacity.
// It initializes size of queue as 0 ( empty queue)
// queue stores satellite_altimeter struct 
FIFOQueue* createFIFO(unsigned capacity)
{
    FIFOQueue* queue = (FIFOQueue*)malloc(sizeof(FIFOQueue));
    queue->capacity = capacity;
    queue->front = queue->size = 0;
 
    // This is important, see the enqueue
    queue->rear = capacity - 1;
    queue->array = (satellite_altimeter*)malloc(queue->capacity * sizeof(satellite_altimeter));
    return queue;
}
 
// Queue is full when size = capacity
int isFull(FIFOQueue* queue)
{
    return (queue->size == queue->capacity);
}
 
// Queue is empty when size is 0
int isEmpty(FIFOQueue* queue)
{
    return (queue->size == 0);
}

// get queue size
int qSize(FIFOQueue* queue)
{
    return (queue->size);
}

 
// Function to add an item to the queue.
// It changes rear and size
void enqueue(FIFOQueue* queue, satellite_altimeter item)
{
    if (isFull(queue))
        return;
    queue->rear = (queue->rear + 1)
                  % queue->capacity;
    queue->array[queue->rear] = item;
    queue->size = queue->size + 1;
    //printf("%d enqueued to queue\n", item);
}
 
// Function to remove an item from queue.
// It changes front and size
satellite_altimeter dequeue(FIFOQueue* queue)
{
    satellite_altimeter item = queue->array[queue->front];
    queue->front = (queue->front + 1)
                   % queue->capacity;
    queue->size = queue->size - 1;
    return item;
}
 
// Function to get front of queue
satellite_altimeter front(FIFOQueue* queue)
{
    return queue->array[queue->front];
}
 
// Function to get rear of queue
satellite_altimeter rear(FIFOQueue* queue)
{
    return queue->array[queue->rear];
}
 

