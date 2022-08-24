#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#ifndef FIFO_QUEUE_H
#define FIFO_QUEUE_H

// struct to represent satellite_altimeter object
typedef struct {
    int reading;
    time_t timestamp;
    int coord[2];
} satellite_altimeter;

// A structure to represent a queue
typedef struct {
    int front, rear, size;
    unsigned capacity;
    satellite_altimeter* array;
}FIFOQueue ;

#define CAPACITY 10 // the capacity of the global array

FIFOQueue* createFIFO(unsigned capacity);
int isFull(FIFOQueue* queue);
int isEmpty(FIFOQueue* queue);
int qSize(FIFOQueue* queue);
void enqueue(FIFOQueue* queue, satellite_altimeter item);
satellite_altimeter dequeue(FIFOQueue* queue);
satellite_altimeter front(FIFOQueue* queue);
satellite_altimeter rear(FIFOQueue* queue);

#endif //FIFO_QUEUE_H
