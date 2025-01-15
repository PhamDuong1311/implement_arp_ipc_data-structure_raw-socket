// Đã test
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "uthash.h"
#include "arp_cache.h"

struct arp_entry *arp_cache = NULL;

void lookup_element_to_cache(const char* ip, const char* mac) {
    struct arp_entry *entry = (struct arp_entry *)malloc(sizeof(struct arp_entry));
    strcpy(entry->ip_addr, ip);
    strcpy(entry->mac_addr, mac);
    entry->timestamp = time(NULL);
    
    HASH_ADD_STR(arp_cache, ip_addr, entry);
}

char* get_element_from_cache(const char* ip) {
    struct arp_entry *entry;
    HASH_FIND_STR(arp_cache, ip, entry);
    if (entry != NULL) {
        return entry->mac_addr; 
    }
    return NULL;  
}
