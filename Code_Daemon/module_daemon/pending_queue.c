#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "pending_queue.h"

void initQueue(Queue *q) {
    q->front = 0;
    q->rear = -1;
    q->size = 0;
}

int isQueueEmpty(Queue *q) {
    return (q->size == 0);
}

int isQueueFull(Queue *q) {
    return (q->size == QUEUE_SIZE);
}

void enqueue(Queue *q, const char *ip, const unsigned char *mac, int count2, int status) {
    if (isQueueFull(q)) {
        printf("Queue is full! Cannot enqueue.\n");
        return;
    }

    q->rear = (q->rear + 1) % QUEUE_SIZE;
    strcpy(q->items[q->rear].ip, ip);
    memcpy(q->items[q->rear].mac, mac, 6);
    q->items[q->rear].timestamp = time(NULL);
    q->items[q->rear].count2 = count2;
    q->items[q->rear].status = status;
    q->size++;
}

void dequeue(Queue *q) {
    if (isQueueEmpty(q)) {
        printf("Queue is empty! Cannot dequeue.\n");
        return;
    }

    q->front = (q->front + 1) % QUEUE_SIZE;
    q->size--;
}

QueueItem* findInQueue(Queue *q, const char *ip) {
    if (isQueueEmpty(q)) return NULL;

    for (int i = 0; i < q->size; i++) {
        int index = (q->front + i) % QUEUE_SIZE;
        if (strcmp(q->items[index].ip, ip) == 0) {
            return &q->items[index];
        }
    }
    return NULL;
}

void displayQueue(Queue *q) {
    if (isQueueEmpty(q)) {
        printf("Queue is empty\n");
        return;
    }

    printf("IP\t\tMAC\t\t\tTimestamp\tCount2\tStatus\n");
    printf("-------------------------------------------------------------\n");

    for (int i = 0; i < q->size; i++) {
        int index = (q->front + i) % QUEUE_SIZE;
        QueueItem *item = &q->items[index];

        printf("%s\t%02X:%02X:%02X:%02X:%02X:%02X\t%ld\t%d\t%d\n",
               item->ip,
               item->mac[0], item->mac[1], item->mac[2],
               item->mac[3], item->mac[4], item->mac[5],
               item->timestamp, item->count2, item->status);
    }
}

// Kiểm tra hoạt động của Queue
int main() {
    Queue q;
    initQueue(&q);

    unsigned char mac1[6] = {0x00, 0x1A, 0x2B, 0x3C, 0x4D, 0x5E};
    unsigned char mac2[6] = {0xFF, 0xEE, 0xDD, 0xCC, 0xBB, 0xAA};

    enqueue(&q, "192.168.1.1", mac1, 5, 1);
    enqueue(&q, "192.168.1.2", mac2, 3, 0);

    printf("\nQueue sau khi thêm phần tử:\n");
    displayQueue(&q);

    QueueItem *found = findInQueue(&q, "192.168.1.1");
    if (found) {
        printf("\nTìm thấy IP: %s, Count2: %d, Status: %d\n", found->ip, found->count2, found->status);
    } else {
        printf("\nKhông tìm thấy IP\n");
    }

    printf("\nQueue sau khi dequeue:\n");
    dequeue(&q);
    displayQueue(&q);

    return 0;
}
