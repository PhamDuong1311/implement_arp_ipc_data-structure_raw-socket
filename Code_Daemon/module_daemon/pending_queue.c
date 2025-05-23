#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <stdint.h>
#include <arpa/inet.h>  
#include <netinet/in.h> 

#include "pending_queue.h"
#include "arp_cache.h"

int is_queue_empty(queue_t *q) {
    return (q->size == 0);
}

int is_queue_full(queue_t *q) {
    return (q->size == QUEUE_SIZE);
}

void enqueue(queue_t *q, const char *ip, const unsigned char *mac, int count2, int status) {
    if (is_queue_full(q)) {
        printf("Queue is full! Cannot enqueue.\n");
        return;
    }
    if (!ip || strlen(ip) == 0) {  
        printf("Error: IP address is empty! Cannot enqueue.\n");
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

void dequeue(queue_t *q) {
    if (is_queue_empty(q)) {
        printf("Queue is empty! Cannot dequeue.\n");
        return;
    }
    q->front = (q->front + 1) % QUEUE_SIZE;
    q->size--;
}


void update_queue(queue_t *q, uint8_t *ip, uint8_t *mac) {
    if (is_queue_empty(q)) {
        return;
    }

    inet_ntop(AF_INET, ip, q->items[q->front].ip, sizeof(q->items[q->front].ip));
    memcpy(q->items[q->front].mac, mac, 6);
    q->items[q->front].status = 1;
}

// int main() {
//     queue_t q;
//     initQueue(&q);

//     unsigned char mac1[6] = {0x00, 0x1A, 0x2B, 0x3C, 0x4D, 0x5E};
//     unsigned char mac2[6] = {0xFF, 0xEE, 0xDD, 0xCC, 0xBB, 0xAA};

//     enqueue(&q, "192.168.1.1", mac1, 5, 1);
//     enqueue(&q, "192.168.1.2", mac2, 3, 0);

//     printf("\nqueue_t sau khi thêm phần tử:\n");
//     display_queue(&q);

//     queue_item_t *found = findInQueue(&q, "192.168.1.1");
//     if (found) {
//         printf("\nTìm thấy IP: %s, Count2: %d, Status: %d\n", found->ip, found->count2, found->status);
//     } else {
//         printf("\nKhông tìm thấy IP\n");
//     }

//     printf("\nqueue_t sau khi dequeue:\n");
//     dequeue(&q);
//     display_queue(&q);

//     return 0;
// }
