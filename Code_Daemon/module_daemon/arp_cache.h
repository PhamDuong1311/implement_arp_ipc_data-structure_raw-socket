#ifndef ARP_CACHE_H
#define ARP_CACHE_H

#include "uthash.h"

#include <time.h>

#define MAX_ARP_CACHE_SIZE 100

extern int arp_cache_size;

struct arp_entry {
    char ip_addr[16];      
    uint8_t mac_addr[6];     
    time_t timestamp;      
    UT_hash_handle hh;     
}; 

extern struct arp_entry *arp_cache_head;
extern struct arp_entry arp_cache[MAX_ARP_CACHE_SIZE];

void lookup_element_to_cache(char* ip, unsigned char* mac);
unsigned char* get_element_from_cache(char* ip);

#endif
