#include <stdio.h>      
#include <stdlib.h>     
#include <string.h>     
#include <stdint.h>     
#include <unistd.h>     
#include <sys/socket.h> 
#include <sys/ioctl.h>  
#include <netinet/in.h> 
#include <net/if.h>     
#include <arpa/inet.h>  
#include <sys/stat.h>   
#include <sys/types.h>  
#include <sys/un.h>
#include "main_thread.h"     

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

int get_mac_address(const char *iface, uint8_t *mac) {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    struct ifreq ifr;

    if (sock < 0) {
        perror("Socket error");
        return -1;
    }

    strncpy(ifr.ifr_name, iface, IFNAMSIZ);
    if (ioctl(sock, SIOCGIFHWADDR, &ifr) < 0) {
        perror("ioctl error");
        close(sock);
        return -1;
    }
    memcpy(mac, ifr.ifr_hwaddr.sa_data, 6);
    close(sock);
    return 0;
}


int get_ip_address(const char *iface, uint8_t *ip) {
    int sock;
    struct ifreq ifr;

    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        perror("Socket error");
        return -1;
    }
    strncpy(ifr.ifr_name, iface, IFNAMSIZ - 1);
    ifr.ifr_name[IFNAMSIZ - 1] = '\0';

    if (ioctl(sock, SIOCGIFADDR, &ifr) < 0) {
        perror("ioctl error");
        close(sock);
        return -1;
    }

    memcpy(ip, &((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr, 4);
    close(sock);
    return 0;
}

void create_daemon() {
    pid_t pid = fork();
    if (pid < 0) {
        perror("Fork failed");
        exit(EXIT_FAILURE);
    }
    if (pid > 0) {
        exit(EXIT_SUCCESS);
    }

    if (setsid() < 0) {
        perror("setsid failed");
        exit(EXIT_FAILURE);
    }

    pid = fork();
    if (pid < 0) {
        perror("Fork failed");
        exit(EXIT_FAILURE);
    }
    if (pid > 0) {
        exit(EXIT_SUCCESS);
    }

    umask(0);
    chdir("/");

    for (int i = 0; i < 3; i++) {
        close(i);
    }
}

int setup_socket() {
    int server_sock;
    struct sockaddr_un server_addr;

    unlink(SOCKET_PATH);

    server_sock = socket(AF_UNIX, SOCK_STREAM, 0);
    if (server_sock == -1) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sun_family = AF_UNIX;
    strncpy(server_addr.sun_path, SOCKET_PATH, sizeof(server_addr.sun_path) - 1);

    if (bind(server_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("bind failed");
        close(server_sock);
        exit(EXIT_FAILURE);
    }

    if (listen(server_sock, 5) == -1) {
        perror("listen failed");
        close(server_sock);
        exit(EXIT_FAILURE);
    }

    return server_sock;
}

void initQueue(Queue *q) {
    q->front = 0;
    q->rear = -1;
    q->size = 0;
}