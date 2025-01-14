#ifndef ARP_CACHE_H
#define ARP_CACHE_H

#include <time.h>

// Định nghĩa cấu trúc ARP Entry
struct arp_entry {
    char ip_addr[16];      // Địa chỉ IP (key)
    char mac_addr[18];     // Địa chỉ MAC
    time_t timestamp;      // Thời điểm lưu entry
    UT_hash_handle hh;     // Cấu trúc hỗ trợ trong uthash
}; 

// Khai báo các hàm
void add_to_cache(const char* ip, const char* mac);
char* get_element_from_cache(const char* ip);
int lookup_element_to_cache(const char* ip);
int is_element_exist(const char* ip);

#endif
