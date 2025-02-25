#ifndef CLI_H
#define CLI_H

#define SOCKET_PATH "/tmp/arp_socket"

int setup_socket();
void send_request(int sockfd, const char *message);
void receive_response(int sockfd);
void show_help();

#endif

