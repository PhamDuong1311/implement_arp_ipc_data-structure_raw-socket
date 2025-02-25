
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
    struct arp_entry *entry;

    HASH_FIND_STR(arp_cache_head, ip, entry);
    if (entry) {
        printf("ARP entry for %s already exists.\n", ip);
        return;
    }

    if (arp_cache_size >= MAX_ARP_CACHE_SIZE) {
        printf("ARP cache full!\n");
        return;
    }

    entry = &arp_cache[arp_cache_size];
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

void show_arp_cache(char *response, size_t response_size) {
    snprintf(response, response_size, "ARP Table:\n");
    struct arp_entry *entry, *tmp;

    HASH_ITER(hh, arp_cache_head, entry, tmp) {
        char entry_str[64];
        snprintf(entry_str, sizeof(entry_str), "%s -> %02X:%02X:%02X:%02X:%02X:%02X\n",
                 entry->ip_addr,
                 entry->mac_addr[0], entry->mac_addr[1], entry->mac_addr[2],
                 entry->mac_addr[3], entry->mac_addr[4], entry->mac_addr[5]);
        strncat(response, entry_str, response_size - strlen(response) - 1);
    }
    printf("Sent ARP table\n");
}


void delete_element_from_cache(char* ip) {
    struct arp_entry *entry;
    
    HASH_FIND_STR(arp_cache_head, ip, entry);
    if (entry) {
        HASH_DEL(arp_cache_head, entry);
        printf("Deleted ARP entry for %s\n", ip);
        arp_cache_size--;
    } else {
        printf("No ARP entry found for %s\n", ip);
    }
}

unsigned char* get_element_from_cache(char* ip) {
    struct arp_entry *entry;
    HASH_FIND_STR(arp_cache_head, ip, entry);
    
    if (entry != NULL) {
        return entry->mac_addr;
    }

    return NULL;
}

