#ifndef ARP_H
#define ARP_H

#include <stdint.h>

extern uint8_t mac_dst[6];
extern uint8_t mac_src[6];
extern uint8_t ip_dst[4];
extern uint8_t ip_src[4];
extern int flag;

struct arp_header {
    uint16_t htype;
    uint16_t ptype;
    uint8_t hlen;
    uint8_t plen;
    uint16_t oper;
    uint8_t sha[6];  
    uint8_t spa[4];  
    uint8_t tha[6];  
    uint8_t tpa[4]; 
};

void send_arp_request(const char *iface, uint8_t *src_mac, uint8_t *src_ip, const char *target_ip);
void send_arp_reply(const char *iface, uint8_t *src_mac, uint8_t *src_ip, uint8_t *target_mac, uint8_t *target_ip);
void receive_arp(const char *iface);
#endif
