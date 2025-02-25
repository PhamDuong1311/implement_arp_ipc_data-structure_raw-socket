#ifndef DAEMON_H
#define DAEMON_H

#define SOCKET_PATH "/tmp/arp_socket"

void create_daemon();
int setup_socket();
void receive_request(int server_sock);
void send_response(int client_sock, const char *message);
void process_request(int client_sock, const char *buffer);

#endif 
