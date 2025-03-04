#ifndef ARP_HELPER_H
#define ARP_HELPER_H

#include "pending_queue.h"
#include "general.h"

void initQueue(Queue *q);
void print_usage(const char *prog_name);
void parse_arguments(int argc, char *argv[], const char **iface, int *cache_timeout);
void disable_kernel_arp(const char *iface);
int get_mac_address(const char *iface, uint8_t *mac);
int get_ip_address(const char *iface, uint8_t *ip);
void create_daemon();
int setup_socket();

#endif 
