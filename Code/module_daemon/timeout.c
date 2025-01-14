#include "timeout.h"
#include "arp_cache.h"

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

int is_element_expire(time_t timestamp) {
    time_t current_time = time(NULL);
    return (current_time - timestamp) > 15; // Example: 15 seconds timeout
}
