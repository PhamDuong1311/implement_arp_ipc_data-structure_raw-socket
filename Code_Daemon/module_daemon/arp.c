/* 
Problem: Muốn implement ARP protocol trên Github Codespaces nhưng
nó có 1 vấn đề đó là Codespace sử dụng môi trường container hóa, vì thế 
Codespaces sẽ không có quyền truy cập vào các card vật lý thật (card WiFi 
hay Ethernet) mà chỉ được setup trên interface ảo (eth0, docker0, lo ảo),
mà mục tiêu đề ra đó là implement ARP trên 1 máy có sử dụng 2 card wifi
=> sử dụng Codespaces bất khả thi => phải cài trực tiếp linux :< => xử lý sau...

Thêm nữa, lý do mặc dù dùng card wifi mà vẫn dùng các thư viện của ethernet bởi vì
packet sau khi đi vào card nó sẽ tự convert wifi header thành ethernet header (bỏ đi các field không cần thiết của wifi)
=> có thể dùng card wifi với các include của ethernet :>
*/

#include "arp.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <linux/if_ether.h>
#include <linux/if_arp.h>

uint8_t mac_target[6] = "";

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
    struct arp_header arp_req;
    struct sockaddr_ll sa;
    int sockfd;

    sockfd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ARP));
    if (sockfd == -1) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&arp_req, 0, sizeof(struct arp_header));

    struct interface_info iface_info = get_src_addr(iface);

    arp_req.hw_type = htons(1);  
    arp_req.pro_type = htons(0x0800);  
    arp_req.hw_length = 6;  
    arp_req.pro_length = 4;  
    arp_req.opcode = htons(1);  
    memcpy(arp_req.sender_mac, iface_info.mac_address, 6);
    memcpy(arp_req.sender_ip, iface_info.ip_addr, 4);
    memset(arp_req.target_mac, 0, 6);  
    inet_pton(AF_INET, target_ip, arp_req.target_ip);

    memset(&sa, 0, sizeof(sa));
    sa.sll_protocol = htons(ETH_P_ARP);
    if (sendto(sockfd, &arp_req, sizeof(arp_req), 0, (struct sockaddr *)&sa, sizeof(sa)) == -1) {
        perror("Send failed");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    printf("ARP Request sent from %s for IP: %s\n", iface, target_ip);
    close(sockfd);
}

void receive_arp_reply(const char *iface) {
    struct arp_header arp_resp;
    int sockfd;
    struct sockaddr_ll sa;
    socklen_t sa_len = sizeof(sa);

    sockfd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ARP));
    if (sockfd == -1) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&sa, 0, sizeof(sa));
    sa.sll_protocol = htons(ETH_P_ARP);

    sa.sll_ifindex = if_nametoindex(iface);
    if (sa.sll_ifindex == 0) {
        perror("Failed to get interface index");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    while (1) {
        int len = recvfrom(sockfd, &arp_resp, sizeof(arp_resp), 0, (struct sockaddr *)&sa, &sa_len);
        if (len < 0) {
            perror("Receive failed");
            close(sockfd);
            exit(EXIT_FAILURE);
        }

        if (ntohs(arp_resp.opcode) == 2) {
            printf("ARP Reply received on %s: IP %d.%d.%d.%d is at MAC ", 
                   iface,
                   arp_resp.sender_ip[0], arp_resp.sender_ip[1],
                   arp_resp.sender_ip[2], arp_resp.sender_ip[3]);
            printf("%02x:%02x:%02x:%02x:%02x:%02x\n", 
                   arp_resp.sender_mac[0], arp_resp.sender_mac[1],
                   arp_resp.sender_mac[2], arp_resp.sender_mac[3],
                   arp_resp.sender_mac[4], arp_resp.sender_mac[5]);
            
            memcpy(mac_target, arp_resp.sender_mac, 6);
            break;  
        }
    }

    close(sockfd);
}