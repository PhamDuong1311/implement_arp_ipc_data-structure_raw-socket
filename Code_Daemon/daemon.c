#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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

pthread_mutex_t cache_mutex = PTHREAD_MUTEX_INITIALIZER;

void *thread_cli_handler(void *arg) {
    int server_sock = *(int *)arg;
    while (1) {
        receive_request(server_sock);
    }
    
    unlink(SOCKET_PATH);
    return NULL;
}

void *thread_send_arp(void *arg) {
    while (1) {
        sleep(1);
    }
    return NULL;
}

void *thread_receive_arp(void *arg) {
    while (1) {
    }
    return NULL;
}
 
void *thread_manage_cache(void *arg) {
    while (1) {
        sleep(5);
    }
    return NULL;
}

int main(int argc, char *argv[]) {
    const char *iface = "wlan0";
    int cache_timeout = 15;
    uint8_t ip_src[4];
    uint8_t mac_src[6];
    int server_sock;
    
    // Địng nghĩa các hàm dưới ở main_thread.c
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
    //create_daemon();
    server_sock = setup_socket();
    // Hết định nghĩa các hàm ở main_thread.c

    // Tạo các Thread
    pthread_t cli_thread, send_thread, recv_thread, cache_thread;

    pthread_create(&cli_thread, NULL, thread_cli_handler, NULL);
    pthread_create(&send_thread, NULL, thread_send_arp, NULL);
    pthread_create(&recv_thread, NULL, thread_receive_arp, NULL);
    pthread_create(&cache_thread, NULL, thread_manage_cache, &cache_timeout);

    pthread_join(cli_thread, NULL);
    pthread_join(send_thread, NULL);
    pthread_join(recv_thread, NULL);
    pthread_join(cache_thread, NULL);
    
    return 0;
}


