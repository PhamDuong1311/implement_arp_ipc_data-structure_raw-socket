#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include "cli.h"

int setup_socket() {
    int sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sockfd == -1) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_un addr = {0};
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SOCKET_PATH, sizeof(addr.sun_path) - 1);

    if (connect(sockfd, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
        perror("connect failed");
        close(sockfd);
        exit(EXIT_FAILURE);
    }
    return sockfd;
}

void send_request(int sockfd, const char *message) {
    if (send(sockfd, message, strlen(message), 0) == -1) {
        perror("send failed");
        close(sockfd);
        exit(EXIT_FAILURE);
    }
}


void receive_response(int sockfd) {
    char buffer[256];
    int bytes_received = recv(sockfd, buffer, sizeof(buffer) - 1, 0);
    if (bytes_received > 0) {
        buffer[bytes_received] = '\0';
        printf("Response from daemon: %s\n", buffer);
    }

}

void show_help() {
    printf("Usage: ./cli [Options]\n");
    printf("Options:\n");
    printf("  -a <IP> <MAC>   Add an entry to the ARP cache\n");
    printf("  -d <IP>         Delete an entry from the ARP cache\n");
    printf("  -s              Show all ARP cache entries\n");
    printf("  -f <IP>         Find MAC address of other device\n);
    printf("  -h              Show help\n");
}

int main(int argc, char *argv[]) {

    int opt;
    char message[256] = {0};
    
    if (argc < 2) {
        show_help();
        return EXIT_FAILURE;
    }

    while ((opt = getopt(argc, argv, "a:d:sf:h")) != -1) {
        int sockfd = setup_socket();  
        switch (opt) {
            case 'a':
                if (optind < argc) {
                    snprintf(message, sizeof(message), "ADD %s %s", optarg, argv[optind]);
                    send_request(sockfd, message);  
                    receive_response(sockfd);
                    optind++;
                } else {
                    fprintf(stderr, "Error: -a requires an IP and MAC address\n");
                    return EXIT_FAILURE;
                }
                break;
            case 'd':
                snprintf(message, sizeof(message), "DELETE %s", optarg);
                send_request(sockfd, message);  
                receive_response(sockfd);
                break;
            case 's':
                strcpy(message, "SHOW");
                send_request(sockfd, message);  
                receive_response(sockfd);
                break;
            case 'f':
                snprintf(message, sizeof(message), "FIND %s", optarg);
                send_request(sockfd, message);  
                receive_response(sockfd);
                break;
            case 'h':
                show_help();
                return EXIT_SUCCESS;
            default:
                show_help();
                return EXIT_FAILURE;
        }
        

        close(sockfd);
    }

    return EXIT_SUCCESS;
}


