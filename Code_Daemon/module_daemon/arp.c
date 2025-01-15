#include "arp.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <netpacket/packet.h>
#include <linux/if_ether.h>
#include <linux/if_arp.h>

struct interface_info get_src_addr(const char *iface) {
    struct ifreq ifr;
    struct interface_info info;
    int sock;

    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock == -1) {
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }

    strncpy(ifr.ifr_name, iface, IFNAMSIZ - 1);
    ifr.ifr_name[IFNAMSIZ - 1] = '\0';

    if (ioctl(sock, SIOCGIFHWADDR, &ifr) == -1) {
        perror("Failed to get MAC address");
        close(sock);
        exit(EXIT_FAILURE);
    }
    memcpy(info.mac_address, ifr.ifr_hwaddr.sa_data, 6);

    if (ioctl(sock, SIOCGIFADDR, &ifr) == -1) {
        perror("Failed to get IP address");
        close(sock);
        exit(EXIT_FAILURE);
    }
    struct sockaddr_in *ipaddr = (struct sockaddr_in *)&ifr.ifr_addr;
    memcpy(info.ip_addr, &ipaddr->sin_addr, 4);

    close(sock);
    return info;
}

void send_arp_request(const char *iface, const char *target_ip) {
    // Implement send ARP request
}

void receive_arp_reply() {
    // Implement receive ARP reply
}
