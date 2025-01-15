#ifndef ARP_CACHE_H
#define ARP_CACHE_H

#include <time.h>

#define MAX_ARP_CACHE_SIZE 100

extern int arp_cache_size;
extern struct arp_entry *arp_cache_head;
extern struct arp_entry arp_cache[MAX_ARP_CACHE_SIZE];

struct arp_entry {
    char ip_addr[16];      
    char mac_addr[18];     
    time_t timestamp;      
    UT_hash_handle hh;     
}; 

void lookup_element_to_cache(const char* ip, const char* mac);
char* get_element_from_cache(const char* ip);

#endif
