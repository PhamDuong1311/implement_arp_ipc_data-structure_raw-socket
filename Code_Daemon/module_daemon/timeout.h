#ifndef TIMEOUT_H
#define TIMEOUT_H

void remove_element_expired(int arp_cache_size, struct arp_entry arp_cache[]);
int is_element_expired(time_t timestamp);
#endif
