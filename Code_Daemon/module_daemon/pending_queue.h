#ifndef QUEUE_H
#define QUEUE_H

#include "general.h"

int is_queue_empty(queue_t *q);
int is_queue_full(queue_t *q);
void enqueue(queue_t *q, const char *ip, const unsigned char *mac, int count2, int status);
void dequeue(queue_t *q);
void update_queue(queue_t *q, uint8_t *ip, uint8_t *mac);

#endif 
