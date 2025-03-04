#ifndef QUEUE_H
#define QUEUE_H

#define QUEUE_SIZE 10  

typedef struct {
    char ip[16];      
    unsigned char mac[6]; 
    time_t timestamp;        
    int count2;             
    int status;              
} QueueItem;

typedef struct {
    QueueItem items[QUEUE_SIZE]; 
    int front, rear, size;       
} Queue;

extern Queue q;

int isQueueEmpty(Queue *q);
int isQueueFull(Queue *q);
void enqueue(Queue *q, const char *ip, const unsigned char *mac, int count2, int status);
void dequeue(Queue *q);
QueueItem* listQueue(Queue *q);
void displayQueue(Queue *q);
void checkQueue(Queue *q);
void updateQueue(Queue *q, uint8_t *ip, uint8_t *mac);

#endif 