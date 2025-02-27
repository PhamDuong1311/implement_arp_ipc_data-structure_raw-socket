#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/ether.h>
#include <netinet/in.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <linux/if_packet.h>
#include "arp.h"



int get_mac_address(const char *iface, uint8_t *mac) {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        perror("Socket error");
        return -1;
    }
    struct ifreq ifr;
    strncpy(ifr.ifr_name, iface, IFNAMSIZ);
    if (ioctl(sock, SIOCGIFHWADDR, &ifr) < 0) {
        perror("ioctl error");
        close(sock);
        return -1;
    }
    memcpy(mac, ifr.ifr_hwaddr.sa_data, 6);
    close(sock);
    return 0;
}


int get_ip_address(const char *iface, uint8_t *ip) {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        perror("Socket error");
        return -1;
    }
    struct ifreq ifr;
    strncpy(ifr.ifr_name, iface, IFNAMSIZ);
    if (ioctl(sock, SIOCGIFADDR, &ifr) < 0) {
        perror("ioctl error");
        close(sock);
        return -1;
    }
    struct sockaddr_in *addr = (struct sockaddr_in *)&ifr.ifr_addr;
    memcpy(ip, &addr->sin_addr, 4);
    close(sock);
    return 0;
}


void send_arp_request(const char *iface, uint8_t *src_mac, uint8_t *src_ip, const char *target_ip) {
    int sockfd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ARP));
    if (sockfd < 0) {
        perror("Socket error");
        exit(1);
    }

    struct ifreq ifr;
    strncpy(ifr.ifr_name, iface, IFNAMSIZ);
    if (ioctl(sockfd, SIOCGIFINDEX, &ifr) < 0) {
        perror("ioctl error");
        close(sockfd);
        exit(1);
    }

    int ifindex = ifr.ifr_ifindex;


    struct ether_header eth;
    struct arp_header arp;
    uint8_t packet[42];  

    memset(eth.ether_dhost, 0xFF, 6); 
    memcpy(eth.ether_shost, src_mac, 6);
    eth.ether_type = htons(ETH_P_ARP);


    arp.htype = htons(1); 
    arp.ptype = htons(ETH_P_IP);
    arp.hlen = 6;
    arp.plen = 4;
    arp.oper = htons(1); 
    memcpy(arp.sha, src_mac, 6);
    memcpy(arp.spa, src_ip, 4);
    memset(arp.tha, 0, 6);
    inet_pton(AF_INET, target_ip, arp.tpa);


    memcpy(packet, &eth, 14);
    memcpy(packet + 14, &arp, 28);

    struct sockaddr_ll socket_address;
    memset(&socket_address, 0, sizeof(socket_address));
    socket_address.sll_ifindex = ifindex;
    socket_address.sll_halen = ETH_ALEN;
    memset(socket_address.sll_addr, 0xFF, 6); 

    if (sendto(sockfd, packet, sizeof(packet), 0, (struct sockaddr *)&socket_address, sizeof(socket_address)) < 0) {
        perror("Sendto error");
    } else {
        printf("ARP Request sent from %s to %s\n", iface, target_ip);
    }

    close(sockfd);
}


void receive_arp_reply(const char *iface) {
    int sockfd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ARP));
    if (sockfd < 0) {
        perror("Socket error");
        exit(1);
    }

    while (1) {
        uint8_t buffer[100];
        struct sockaddr_ll addr;
        socklen_t addr_len = sizeof(addr);
        int len = recvfrom(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr *)&addr, &addr_len);
        if (len < 0) {
            perror("Recvfrom error");
            continue;
        }

        struct arp_header *arp = (struct arp_header *)(buffer + 14);
        if (ntohs(arp->oper) == 2) { 
            printf("Received ARP Reply:\n");
            printf("Sender MAC: %02x:%02x:%02x:%02x:%02x:%02x\n",
                   arp->tha[0], arp->tha[1], arp->tha[2], arp->tha[3], arp->tha[4], arp->tha[5]);
            printf("Sender IP: %d.%d.%d.%d\n", arp->tpa[0], arp->tpa[1], arp->tpa[2], arp->tpa[3]);
            break;
        }
    }
    close(sockfd);
}

