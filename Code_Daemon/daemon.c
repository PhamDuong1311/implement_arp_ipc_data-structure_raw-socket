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
#include "module_daemon/timeout.h"
#include "module_daemon/arp.h"

pthread_mutex_t cache_mutex = PTHREAD_MUTEX_INITIALIZER;

void print_usage(const char *prog_name) {
    printf("Usage: %s -i <network_interface> -t <cache_timeout>\n", prog_name);
    printf("Options:\n");
    printf("  -i <interface>       Network interface to disable ARP\n");
    printf("  -t <timeout>         ARP cache timeout in seconds\n");
    printf("  -h                   Show this help message\n");
}

void parse_arguments(int argc, char *argv[], const char **iface, int *cache_timeout) {
    int opt;
    while ((opt = getopt(argc, argv, "i:t:h")) != -1) {
        switch (opt) {
            case 'i':
                *iface = optarg;
                break;
            case 't':
                *cache_timeout = atoi(optarg);
                break;
            case 'h':
                print_usage(argv[0]);
                exit(EXIT_SUCCESS);
            default:
                print_usage(argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    if (!*iface) {
        fprintf(stderr, "Error: Network interface is required.\n");
        print_usage(argv[0]);
        exit(EXIT_FAILURE);
    }
}

void disable_kernel_arp(const char *iface) {
    char command[128];
    snprintf(command, sizeof(command), "ip link set dev %s arp off", iface);
    system(command);
    printf("ARP disabled on interface: %s\n", iface);
}

void *thread_cli_handler(void *arg) {
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
    
    // Cấu hình chọn interface, timeout
    parse_arguments(argc, argv, &iface, &cache_timeout);
    
    // Lấy IP, MAC của interface
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
    
    // Tắt xử lý ARP ở kernel
    disable_kernel_arp(iface);
    
    // Khởi tạo daemon, IPC
    //create_daemon();
    int server_sock = setup_socket();
    
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


