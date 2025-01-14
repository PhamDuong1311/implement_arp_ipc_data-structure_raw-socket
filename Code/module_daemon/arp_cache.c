#include "arp_cache.h"
#include <stdio.h>
#include <string.h>
#include <time.h>

#define ARP_CACHE_SIZE 100
static struct arp_cache_entry arp_cache[ARP_CACHE_SIZE];
static int arp_cache_size = 0;

void add_element_to_cache(const char *ip, const char *mac) {
    if (arp_cache_size < ARP_CACHE_SIZE) {
        strncpy(arp_cache[arp_cache_size].ip, ip, 16);
        strncpy(arp_cache[arp_cache_size].mac, mac, 18);
        arp_cache[arp_cache_size].timestamp = time(NULL);
        arp_cache_size++;
    }
}

int is_entry_expired(time_t timestamp) {
    time_t now = time(NULL);
    return (now - timestamp) > 15; // 15 seconds expiration
}

void remove_element_expire() {
    for (int i = 0; i < arp_cache_size; i++) {
        if (is_entry_expired(arp_cache[i].timestamp)) {
            for (int j = i; j < arp_cache_size - 1; j++) {
                arp_cache[j] = arp_cache[j + 1];
            }
            arp_cache_size--;
            i--;
        }
    }
}
