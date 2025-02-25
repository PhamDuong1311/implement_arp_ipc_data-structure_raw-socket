#ifndef ARP_H
#define ARP_H

#include <stdint.h>

extern uint8_t mac_target[6];

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



int get_mac_address(const char *iface, uint8_t *mac);
int get_ip_address(const char *iface, uint8_t *ip);
void send_arp_request(const char *ifaceA, const char *ifaceB, const char *target_ip);
void receive_arp_reply(const char *ifaceB);
#endif
