#ifndef ARP_H
#define ARP_H

#include "general.h"

void send_arp_request(const char *iface, uint8_t *src_mac, uint8_t *src_ip, const char *target_ip);
void send_arp_reply(const char *iface, uint8_t *src_mac, uint8_t *src_ip, uint8_t *target_mac, uint8_t *target_ip);
void receive_arp(const char *iface);
#endif
