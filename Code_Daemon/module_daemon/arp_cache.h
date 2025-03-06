#ifndef ARP_CACHE_H
#define ARP_CACHE_H

#include "general.h"

void lookup_element_to_cache(char* ip, unsigned char* mac);//ok
void show_arp_cache(char *response, size_t response_size);//ok
void delete_element_from_cache(char* ip);//ok
unsigned char* get_element_from_cache(char* ip);//ok
void remove_entry_expired(int cache_timeout);//ok
void save_cache_to_file();



#endif
