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

int receive_request(int server_sock) {
    struct sockaddr_un client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    int client_sock;
    int bytes_received;

    client_sock = accept(server_sock, (struct sockaddr *)&client_addr, &client_addr_len);
    if (client_sock == -1) {
        perror("accept failed");
        return -1;  
    }
    memset(buffer, 0, sizeof(buffer));
    bytes_received = recv(client_sock, buffer, sizeof(buffer) - 1, 0);
    if (bytes_received > 0) {
        buffer[bytes_received] = '\0';
        return client_sock;  
    } else {
        printf("Error: Received empty data or failed\n");
        close(client_sock);
        return -1;
    }
}

void send_response(int client_sock, const char *message) {
    if (send(client_sock, message, strlen(message), 0) == -1) {
        perror("send failed");
    }
    close(client_sock);
}

void process_request(int client_sock, const char *buffer) {
    char mac_str[18];
    unsigned char mac[6];
    char response[1024];

    printf("\n------------------------------------------------------\n");
    printf("Received command: %s\n", buffer);

    if (strncmp(buffer, "ADD", 3) == 0) {
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
        if (sscanf(buffer, "DELETE %15s", ip) == 1) {
            delete_element_from_cache(ip);
            send_response(client_sock, "ARP entry deleted");
        } else {
            send_response(client_sock, "Invalid DELETE command format");
        }
    } else if (strncmp(buffer, "FIND", 4) == 0) {
        if (sscanf(buffer, "FIND %15s", ip) == 1) {
            uint8_t *mac_found = get_element_from_cache(ip);
            if (mac_found != NULL) {
		    memcpy(mac, mac_found, 6);
	            exist_mac = 1;
		    snprintf(mac_str, sizeof(mac_str), "%02X:%02X:%02X:%02X:%02X:%02X",
		             mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
		    send_response(client_sock, mac_str);

	    } else {
		exist_mac = 0;
	    }
        } else {
            send_response(client_sock, "Invalid FIND command format");
        }
    } else if (strcmp(buffer, "SHOW") == 0) {
        show_arp_cache(response, 1023);
        send_response(client_sock, response);
    } else {
        send_response(client_sock, "Unknown command");
    }
}

// int main() {
//     int server_sock = setup_socket();
//     int client_sock;
//     while (1) {
//         client_sock = receive_request(server_sock);
//         if (client_sock != -1) {  
//             process_request(client_sock, buffer);
//         }
//     }

//     return 0;
// }
