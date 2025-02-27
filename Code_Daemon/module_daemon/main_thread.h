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

// Hiển thị hướng dẫn sử dụng chương trình
void print_usage(const char *prog_name);

// Phân tích đối số dòng lệnh
void parse_arguments(int argc, char *argv[], const char **iface, int *cache_timeout);

// Vô hiệu hóa ARP trên giao diện mạng
void disable_kernel_arp(const char *iface);

// Lấy địa chỉ MAC của giao diện mạng
int get_mac_address(const char *iface, uint8_t *mac);

// Lấy địa chỉ IP của giao diện mạng
int get_ip_address(const char *iface, uint8_t *ip);

// Chuyển tiến trình thành daemon
void create_daemon();

// Thiết lập socket Unix Domain
int setup_socket();

#endif 
