#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "ipc_daemon.h"
#include "arp_cache.h"


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

void send_response(int client_sock, const char *message) {
    if (send(client_sock, message, strlen(message), 0) == -1) {
        perror("send failed");
        close(client_sock);
        exit(EXIT_FAILURE);
    }
    close(client_sock);    
}

void process_request(int client_sock, const char *buffer) {
    printf ("\n------------------------------------------------------\n");
    printf("Received command: %s\n", buffer);

    if (strncmp(buffer, "ADD", 3) == 0) {
        char ip[16], mac_str[18];
        unsigned char mac[6];

        if (sscanf(buffer, "ADD %15s %17s", ip, mac_str) == 2) {
            if (sscanf(mac_str, "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx",
                       &mac[0], &mac[1], &mac[2], &mac[3], &mac[4], &mac[5]) == 6) {
                lookup_element_to_cache(ip, mac);
                send_response(client_sock, "ARP entry added");
            } else {
                printf("Invalid MAC address format\n");
                send_response(client_sock, "Invalid MAC address format");
            }
        } else {
            printf("Invalid ADD command format\n");
            send_response(client_sock, "Invalid ADD command format");
        }
    } else if (strncmp(buffer, "DELETE", 6) == 0) {
        char ip[16];
        if (sscanf(buffer, "DELETE %15s", ip) == 1) {
            delete_element_from_cache(ip);
            send_response(client_sock, "ARP entry deleted");
        } else {
            send_response(client_sock, "Invalid DELETE command format");
        }
    } else if (strcmp(buffer, "SHOW") == 0) {
        
        char response[1024];
        show_arp_cache(response, 1023);
        send_response(client_sock, response);
    } else {
        send_response(client_sock, "Unknown command");
    }
}


void receive_request(int server_sock) {
    struct sockaddr_un client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    int client_sock;
    
    while (1) {
        client_sock = accept(server_sock, (struct sockaddr *)&client_addr, &client_addr_len);
        if (client_sock == -1) {
            perror("accept failed");
            continue;
        }

        char buffer[256];
        int bytes_received = recv(client_sock, buffer, sizeof(buffer) - 1, 0);
        if (bytes_received > 0) {
            buffer[bytes_received] = '\0';
            process_request(client_sock, buffer);
        }

        close(client_sock);
    }
}


