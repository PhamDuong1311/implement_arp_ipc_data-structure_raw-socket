#ifndef CLI_H
#define CLI_H

#define SOCKET_PATH "/tmp/arp_socket"

typedef struct {
    char cmd;
    char ip[16];
    char mac[18];
} msg_t;

int setup_socket();
void send_request(int sockfd, msg_t *query);
void receive_response(int sockfd);
void show_help();

#endif

