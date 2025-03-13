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
    bytes_received = recv(client_sock, &msg_from_cli, sizeof(msg_from_cli) - 1, 0);
    if (bytes_received > 0) {
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

void process_request(int client_sock, msg_t *msg_from_cli) {
    char mac_str[18];
    unsigned char mac[6];
    unsigned char ip[4];
    char response[1024];

    printf("\n------------------------------------------------------\n");
    printf("Received command: %c %s %s\n", msg_from_cli->cmd, msg_from_cli->ip, msg_from_cli->mac);
    printf("MAC Address received: '%s'\n", msg_from_cli->mac);  // In địa chỉ MAC nhận được

    if (msg_from_cli->cmd == 'a') {  
        if (inet_pton(AF_INET, msg_from_cli->ip, &ip) == 1) { 
            if (sscanf(msg_from_cli->mac, "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx",
                       &mac[0], &mac[1], &mac[2], &mac[3], &mac[4], &mac[5]) == 6) {
                lookup_element_to_cache(msg_from_cli->ip, mac);
                send_response(client_sock, "ARP entry added");
            } else {
                send_response(client_sock, "Invalid MAC address format");
            }
        } else {
            send_response(client_sock, "Invalid IP address format");
        }
    } else if (msg_from_cli->cmd == 'd') {
        if (inet_pton(AF_INET, msg_from_cli->ip, &ip) == 1) {
            delete_element_from_cache(msg_from_cli->ip);
            send_response(client_sock, "ARP entry deleted");
        } else {
            send_response(client_sock, "Invalid IP address format");
        }
    } else if (msg_from_cli->cmd == 'f') {
        if (inet_pton(AF_INET, msg_from_cli->ip, &ip) == 1) {
            uint8_t *mac_found = get_element_from_cache(msg_from_cli->ip);
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
            send_response(client_sock, "Invalid IP address format");
        }
    } else if (msg_from_cli->cmd == 's') {
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
