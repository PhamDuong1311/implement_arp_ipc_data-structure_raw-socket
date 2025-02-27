#ifndef ARP_HELPER_H
#define ARP_HELPER_H

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

#define SOCKET_PATH "/tmp/arp_socket"

void print_usage(const char *prog_name);
void parse_arguments(int argc, char *argv[], const char **iface, int *cache_timeout);
void disable_kernel_arp(const char *iface);
int get_mac_address(const char *iface, uint8_t *mac);
int get_ip_address(const char *iface, uint8_t *ip);
void create_daemon();
int setup_socket();

#endif 
