#ifndef ARP_CACHE_H
#define ARP_CACHE_H

#include <time.h>

struct arp_cache_entry {
    char ip[16];
    char mac[18];
    time_t timestamp;
};

void add_element_to_cache(const char *ip, const char *mac);
int is_entry_expired(time_t timestamp);
void remove_element_expire();

#endif
