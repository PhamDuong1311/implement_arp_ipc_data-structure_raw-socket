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

void lookup_element_to_cache(char* ip, unsigned char* mac) {
    if (arp_cache_size >= MAX_ARP_CACHE_SIZE) {
        printf("ARP cache full!\n");
        return;
    }

    struct arp_entry *entry = &arp_cache[arp_cache_size];
    memset(entry, 0, sizeof(struct arp_entry));

    strncpy(entry->ip_addr, ip, sizeof(entry->ip_addr) - 1);
    entry->ip_addr[sizeof(entry->ip_addr) - 1] = '\0';  

    memcpy(entry->mac_addr, mac, 6);
    entry->timestamp = time(NULL);

    HASH_ADD_STR(arp_cache_head, ip_addr, entry);  
    arp_cache_size++;

    printf("Added MAC for %s: %02X:%02X:%02X:%02X:%02X:%02X\n",
           entry->ip_addr,
           entry->mac_addr[0], entry->mac_addr[1], entry->mac_addr[2],
           entry->mac_addr[3], entry->mac_addr[4], entry->mac_addr[5]);
}


unsigned char* get_element_from_cache(char* ip) {
    struct arp_entry *entry;
    HASH_FIND_STR(arp_cache_head, ip, entry);
    
    if (entry != NULL) {
        return entry->mac_addr;
    }

    return NULL;
}

