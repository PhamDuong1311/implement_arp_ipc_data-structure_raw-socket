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
#include "module_daemon/general.h"

int arp_cache_size = 0;
struct arp_entry *arp_cache_head = NULL;
struct arp_entry arp_cache[MAX_ARP_CACHE_SIZE];

uint8_t mac_dst[6] = {0};
uint8_t ip_dst[4] = {0};
uint8_t mac_src[6] = {0};
uint8_t ip_src[4] = {0};
int flag = 0;

char buffer[256] = {0};
int exist_mac = 1; 
char ip_str[16] = {0};

queue_t q = {};

pthread_mutex_t cache_mutex = PTHREAD_MUTEX_INITIALIZER;

void *thread_cli_handler(void *arg) {
    int server_sock = *(int *) arg;
    int client_sock;
    unsigned char mac[6], mac_default[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};  
    int count1 = 0;
    char mac_str[18];
    
    while (1) {
        client_sock = receive_request(server_sock);
        if (client_sock != -1) {  
            process_request(client_sock, buffer);
        }
        if (exist_mac == 0) {
            pthread_mutex_lock(&cache_mutex);
            enqueue(&q, ip_str, mac_default, 0, 0);
            pthread_mutex_unlock(&cache_mutex);
            while (count1 < 5) {
                sleep(5);
                uint8_t *mac_found = get_element_from_cache(ip_str);
                if (mac_found != NULL) {
                    exist_mac = 1;
                    memcpy(mac, mac_found, 6);
                    snprintf(mac_str, sizeof(mac_str), "%02X:%02X:%02X:%02X:%02X:%02X",
                             mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
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
    char *iface = (char *)arg;

    while (1) {
        pthread_mutex_lock(&cache_mutex);
        queue_item_t *item = list_queue(&q);
        pthread_mutex_unlock(&cache_mutex);

        if (item) {
            if (item->status == 1) {  
                pthread_mutex_lock(&cache_mutex);
                lookup_element_to_cache(item->ip, item->mac);
                dequeue(&q);
                pthread_mutex_unlock(&cache_mutex);
            } else if (item->count2 < 5) {
                send_arp_request(iface, mac_src, ip_src, item->ip);

                pthread_mutex_lock(&cache_mutex);
                item->count2++;
                pthread_mutex_unlock(&cache_mutex);
            } else {
                printf("Max retries reached for %s, removing from queue.\n", item->ip);
                pthread_mutex_lock(&cache_mutex);
                dequeue(&q);
                pthread_mutex_unlock(&cache_mutex);
            }
        }

        sleep(2);
    }
    return NULL;
}


void *thread_receive_arp(void *arg) {
    char *iface = (char *) arg;

    while (1) {
        receive_arp(iface);
        pthread_mutex_lock(&cache_mutex);
        if (flag == 2) {
            update_queue(&q, ip_dst, mac_dst);
            flag = 0;
        }
        if (flag == 1) {
            send_arp_reply(iface, mac_src, ip_src, mac_dst, ip_dst);
            flag = 0;
        }
        pthread_mutex_unlock(&cache_mutex);
        sleep(1);
    }
    return NULL;
}

void *thread_manage_cache(void *arg) {
    int cache_timeout = *(int *) arg;
    while (1) {
        pthread_mutex_lock(&cache_mutex);
        save_cache_to_file();
        remove_entry_expired(cache_timeout);
        pthread_mutex_unlock(&cache_mutex);
        sleep(5);
    }
    return NULL;
}

int main(int argc, char *argv[]) {
    const char *iface = "wlp44s0";
    int cache_timeout = 100;
    int server_sock;
    
    parse_arguments(argc, argv, &iface, &cache_timeout);
    sleep(1);
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
    //disable_kernel_arp(iface);
    server_sock = setup_socket();
    //create_daemon();
    init_queue(&q);
    
    pthread_t cli_thread, send_thread, recv_thread, cache_thread;
    
    pthread_create(&cli_thread, NULL, thread_cli_handler, &server_sock);
    pthread_create(&send_thread, NULL, thread_send_arp, (void *)iface);
    pthread_create(&recv_thread, NULL, thread_receive_arp, (void *)iface);

    pthread_create(&cache_thread, NULL, thread_manage_cache, &cache_timeout);
    
    pthread_join(cli_thread, NULL);
    pthread_join(send_thread, NULL);
    pthread_join(recv_thread, NULL);
    pthread_join(cache_thread, NULL);
    
    return 0;
}
