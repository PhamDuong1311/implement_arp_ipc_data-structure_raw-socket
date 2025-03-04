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
#include "main_thread.h"

uint8_t ip_dst[4] = {};
uint8_t mac_dst[6] = {};
int flag = 0;

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

void send_arp_reply(const char *iface, uint8_t *src_mac, uint8_t *src_ip, uint8_t *target_mac, uint8_t *target_ip) {
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

    memcpy(eth.ether_dhost, target_mac, 6);
    memcpy(eth.ether_shost, src_mac, 6);
    eth.ether_type = htons(ETH_P_ARP);

    arp.htype = htons(1);
    arp.ptype = htons(ETH_P_IP);
    arp.hlen = 6;
    arp.plen = 4;
    arp.oper = htons(2); 
    memcpy(arp.sha, src_mac, 6);
    memcpy(arp.spa, src_ip, 4);
    memcpy(arp.tha, target_mac, 6);
    memcpy(arp.tpa, target_ip, 4);

    memcpy(packet, &eth, 14);
    memcpy(packet + 14, &arp, 28);

    struct sockaddr_ll socket_address;
    memset(&socket_address, 0, sizeof(socket_address));
    socket_address.sll_ifindex = ifindex;
    socket_address.sll_halen = ETH_ALEN;
    memcpy(socket_address.sll_addr, target_mac, 6); 

    if (sendto(sockfd, packet, sizeof(packet), 0, (struct sockaddr *)&socket_address, sizeof(socket_address)) < 0) {
        perror("Sendto error");
    } else {
        printf("ARP Reply sent to %02X:%02X:%02X:%02X:%02X:%02X\n",
               target_mac[0], target_mac[1], target_mac[2], target_mac[3], target_mac[4], target_mac[5]);
    }

    close(sockfd);
}

void receive_arp(const char *iface) {
    int sockfd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ARP));
    if (sockfd < 0) {
        perror("Socket error");
        exit(1);
    }

    while (1) {
        uint8_t msg[100];
        struct sockaddr_ll addr;
        socklen_t addr_len = sizeof(addr);
        int len = recvfrom(sockfd, msg, sizeof(msg), 0, (struct sockaddr *)&addr, &addr_len);
        if (len < 0) {
            perror("Recvfrom error");
            continue;
        }

        struct ether_header *eth = (struct ether_header *)msg;
        struct arp_header *arp = (struct arp_header *)(msg + 14);

        if (ntohs(eth->ether_type) == ETH_P_ARP) {
            if (ntohs(arp->oper) == 1) {  
                printf("[ARP Request] From: %d.%d.%d.%d (%02X:%02X:%02X:%02X:%02X:%02X)\n",
                       arp->spa[0], arp->spa[1], arp->spa[2], arp->spa[3],
                       arp->sha[0], arp->sha[1], arp->sha[2], arp->sha[3], arp->sha[4], arp->sha[5]);
                flag = 1;
                memcpy(ip_dst, arp->spa, 4);
                memcpy(mac_dst, arp->sha, 6);
            } else if (ntohs(arp->oper) == 2) {  
                printf("[ARP Reply] From: %d.%d.%d.%d (%02X:%02X:%02X:%02X:%02X:%02X)\n",
                       arp->spa[0], arp->spa[1], arp->spa[2], arp->spa[3],
                       arp->sha[0], arp->sha[1], arp->sha[2], arp->sha[3], arp->sha[4], arp->sha[5]);
                
                memcpy(ip_dst, arp->spa, 4);
                memcpy(mac_dst, arp->sha, 6);
                flag = 2;
            }
        }
    }

    close(sockfd);
}


// int main() {
//     char iface[] = "wlp44s0";
//     uint8_t src_ip[4];
//     uint8_t src_mac[6];
//     char target_ip[] = "192.168.1.16";
//     if (get_ip_address(iface, src_ip) != 0) {
//         perror("get_ip_address failed");
//         return -1;
//     }

//     if (get_mac_address(iface, src_mac) != 0) {
//         perror("get_mac_address failed");
//         return -1;
//     }

//     send_arp_request(iface, src_mac, src_ip, target_ip);
//     receive_arp_reply(iface);

//     return 0;
// }