#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <getopt.h>
#include "module_daemon/arp_cache.h"
#include "module_daemon/ipc_daemon.h"
#include "module_daemon/arp.h"
#include "module_daemon/main_thread.h"
#include "module_daemon/pending_queue.h"


struct thread_args {
    int server_sock;
    Queue *queue;
};

struct thread_arp_args {
    Queue *q;
    const char *iface;
    uint8_t *src_mac;
    uint8_t *src_ip;
};

typedef struct {
    Queue *q;
    uint8_t ip_src[4];
    uint8_t mac_src[6];
} ThreadArgs;

char buffer[256];
int exist_mac;
uint8_t ip_dst[4];
uint8_t mac_dst[6];

pthread_mutex_t cache_mutex = PTHREAD_MUTEX_INITIALIZER;

void *thread_cli_handler(void *arg) {
    struct thread_args *args = (struct thread_args *)arg;

    int server_sock = args->server_sock;
    Queue *q = args->queue;
    int client_sock;
    unsigned char mac[6], mac_default[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};  
    int count1 = 0;
    char mac_str[256];
    char ip[16];

    while (1) {
        client_sock = receive_request(server_sock);
        if (client_sock != -1) {  
            process_request(client_sock, buffer);
        }

        if (exist_mac == 0) {
            enqueue(q, ip, mac_default, 0, 0);
            while (count1 < 5) {
                sleep(2);
                memcpy(mac, get_element_from_cache(ip), 6);
                if (mac != NULL) {
                    exist_mac = 1;
                    snprintf(mac_str, sizeof(mac_str), "%02X:%02X:%02X:%02X:%02X:%02X",
                             mac[0], mac[1], mac[2],
                             mac[3], mac[4], mac[5]);
                    send_response(client_sock, mac_str);
                    break;
                } else {
                    exist_mac = 0;
                    count1++;
                }
            }

            if (count1 >= 5) {
                send_response(client_sock, "Failed unable to find MAC (count1 limit)");
                count1 = 0;
            }
        }
    }
    
    unlink(SOCKET_PATH);
    return NULL;
}

void *thread_send_arp(void *arg) {
    struct thread_arp_args *args = (struct thread_arp_args *)arg;

    int count3 = 0;
    Queue *q = args->q;
    const char *iface = args->iface;
    uint8_t *src_mac = args->src_mac;
    uint8_t *src_ip = args->src_ip;
    
    while (1) {
        pthread_mutex_lock(&cache_mutex);
        QueueItem *item;
        item = listQueue(q);
        pthread_mutex_unlock(&cache_mutex);
                
        while (count3 <= 5) {
            send_arp_request(iface, src_mac, src_ip, item->ip);
            count3++;
            sleep(1);
        }
        checkQueue(q);
        
    }
    return NULL;
}

void *thread_receive_arp(void *arg) {
    const char *iface = (const char *) arg;
    ThreadArgs *args = (ThreadArgs *)arg; 
    Queue *q = args->q;
    uint8_t *ip_src = args->ip_src;
    uint8_t *mac_src = args->mac_src;

    while (1) {
        receive_arp(iface);
        if (flag == 2) {
            updateQueue(q, ip_dst, mac_dst);
        }

        if (flag == 1) {
            send_arp_reply(iface, mac_src, ip_src, mac_dst, ip_dst);
        }

    }
    return NULL;
}
 
void *thread_manage_cache(void *arg) {
    while (1) {
        remove_element_expired();
    }
    return NULL;
}

int main(int argc, char *argv[]) {
    const char *iface = "wlan0";
    int cache_timeout = 15;
    uint8_t ip_src[4];
    uint8_t mac_src[6];
    int server_sock;
    Queue q;
    
    parse_arguments(argc, argv, &iface, &cache_timeout);
    if (get_mac_address(iface, mac_src) != 0) {
        printf("Failed to get MAC address\n");
    } else {
        printf("MAC address of %s: %02x:%02x:%02x:%02x:%02x:%02x\n", iface,
            mac_src[0], mac_src[1], mac_src[2], mac_src[3], mac_src[4], mac_src[5]);
    }
    if (get_ip_address(iface, ip_src) != 0) {
        printf("Failed to get IP address\n");
    } else {
        printf("IP address of %s: %u.%u.%u.%u\n", iface,
            ip_src[0], ip_src[1], ip_src[2], ip_src[3]);
    }
    disable_kernel_arp(iface);
    server_sock = setup_socket();
    initQueue(&q);
    
    pthread_t cli_thread, send_thread, recv_thread, cache_thread;
    struct thread_args cli_args = {server_sock, &q};
    struct thread_arp_args send_args = {&q, iface, mac_src, ip_src};
    
    pthread_create(&cli_thread, NULL, thread_cli_handler, &cli_args);
    pthread_create(&send_thread, NULL, thread_send_arp, &send_args);
    pthread_create(&recv_thread, NULL, thread_receive_arp, NULL);
    pthread_create(&cache_thread, NULL, thread_manage_cache, &cache_timeout);
    
    pthread_join(cli_thread, NULL);
    pthread_join(send_thread, NULL);
    pthread_join(recv_thread, NULL);
    pthread_join(cache_thread, NULL);
    
    return 0;
}
