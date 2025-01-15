#include "timeout.h"
#include "arp_cache.h"
#include <time.h>

struct arp_entry arp_cache[MAX_ARP_CACHE_SIZE];
int arp_cache_size = 0;

void remove_element_expired(int *arp_cache_size, struct arp_entry arp_cache[]) {
    for (int i = 0; i < *arp_cache_size; i++) {
        if (is_element_expired(arp_cache[i].timestamp)) {
            for (int j = i; j < *arp_cache_size - 1; j++) {
                arp_cache[j] = arp_cache[j + 1];
            }
            (*arp_cache_size)--;
            i--;
        }
    }
}

int is_element_expired(time_t timestamp) {
    time_t current_time = time(NULL);
    return (current_time - timestamp) > 15; 
}
