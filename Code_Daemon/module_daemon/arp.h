#ifndef ARP_H
#define ARP_H

#include <stdint.h>

extern uint8_t mac_target[6];

struct arp_header {
    uint16_t hw_type;
    uint16_t pro_type;
    uint8_t hw_length;
    uint8_t pro_length;
    uint16_t opcode;
    uint8_t sender_ip[4];
    uint8_t sender_mac[6];
    uint8_t target_ip[4];
    uint8_t target_mac[6];
};

struct interface_info {
    uint8_t ip_addr[4];
    uint8_t mac_address[6];
};

struct interface_info get_src_addr(const char *iface);
void send_arp_request(const char *iface, const char *target_ip);
void receive_arp_reply(const char *iface);
#endif
