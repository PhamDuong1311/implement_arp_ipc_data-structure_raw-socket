// Đã test
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "uthash.h"
#include "arp_cache.h"

int arp_cache_size = 0;

struct arp_entry *arp_cache_head = NULL;

struct arp_entry arp_cache[MAX_ARP_CACHE_SIZE];

void lookup_element_to_cache(const char* ip, const char* mac) {
    struct arp_entry *entry = &arp_cache[arp_cache_size];
    strcpy(entry->ip_addr, ip);
    strcpy(entry->mac_addr, mac);
    entry->timestamp = time(NULL);
    
    arp_cache_size++;
    HASH_ADD_STR(arp_cache_head, ip_addr, entry);
}

char* get_element_from_cache(const char* ip) {
    struct arp_entry *entry;
    HASH_FIND_STR(arp_cache_head, ip, entry);
    if (entry != NULL) {
        return entry->mac_addr; 
    }
    return NULL;  
}

