#ifndef DAEMON_H
#define DAEMON_H

#include "arp_cache.h"
#include "main_thread.h"
#include "general.h"

int receive_request(int server_sock);//ok
void send_response(int client_sock, const char *message);//ok
void process_request(int client_sock, const char *buffer);//ok

#endif 
