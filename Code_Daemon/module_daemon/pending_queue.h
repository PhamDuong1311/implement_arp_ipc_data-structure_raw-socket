#ifndef QUEUE_H
#define QUEUE_H

#include "general.h"

int isQueueEmpty(Queue *q);
int isQueueFull(Queue *q);
void enqueue(Queue *q, const char *ip, const unsigned char *mac, int count2, int status);
void dequeue(Queue *q);
QueueItem* listQueue(Queue *q);
void displayQueue(Queue *q);
void checkQueue(Queue *q);
void updateQueue(Queue *q, uint8_t *ip, uint8_t *mac);

#endif 