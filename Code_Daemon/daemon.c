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

pthread_mutex_t cache_mutex = PTHREAD_MUTEX_INITIALIZER;

void *thread_cli_handler(void *arg) {
    int server_sock = *(int *) arg;
    int client_sock;
    unsigned char mac[6], mac_default[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};  
    int count1 = 0;
    char mac_str[18];
    char ip_str[16];

    while (1) {
        client_sock = receive_request(server_sock);
        if (client_sock != -1) {  
            process_request(client_sock, buffer);
        }

        if (exist_mac == 0) {
            enqueue(&q, ip_str, mac_default, 0, 0);
            while (count1 < 5) {
                sleep(2);
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
    const char *iface = (const char *) arg;
    int count3 = 0;
    
    while (1) {
        pthread_mutex_lock(&cache_mutex);
        QueueItem *item = listQueue(&q);
        pthread_mutex_unlock(&cache_mutex);
        
        if (item) {
            while (count3 <= 5) {
                send_arp_request(iface, mac_src, ip_src, item->ip);
                count3++;
                sleep(1);
            }
        }
        checkQueue(&q);
    }
    return NULL;
}

void *thread_receive_arp(void *arg) {
    const char *iface = (const char *) arg;

    while (1) {
        receive_arp(iface);
        if (flag == 2) {
            updateQueue(&q, ip_dst, mac_dst);
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
    int server_sock;
    
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
    
    pthread_create(&cli_thread, NULL, thread_cli_handler, &server_sock);
    pthread_create(&send_thread, NULL, thread_send_arp, &iface);
    pthread_create(&recv_thread, NULL, thread_receive_arp, &iface);
    pthread_create(&cache_thread, NULL, thread_manage_cache, NULL);
    
    pthread_join(cli_thread, NULL);
    pthread_join(send_thread, NULL);
    pthread_join(recv_thread, NULL);
    pthread_join(cache_thread, NULL);
    
    return 0;
}
