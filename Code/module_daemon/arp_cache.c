#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "uthash.h"
#include "arp_cache.h"

// Khai báo biến toàn cục là bảng hash chứa các arp_entry
struct arp_entry *arp_cache = NULL;

// Hàm thêm entry vào cache
void add_to_cache(const char* ip, const char* mac) {
    struct arp_entry *entry = (struct arp_entry *)malloc(sizeof(struct arp_entry));
    strcpy(entry->ip_addr, ip);
    strcpy(entry->mac_addr, mac);
    entry->timestamp = time(NULL);
    
    // Thêm entry vào cache bằng uthash
    HASH_ADD_STR(arp_cache, ip_addr, entry);
}

// Hàm lấy phần tử từ cache
char* get_element_from_cache(const char* ip) {
    struct arp_entry *entry;
    
    // Tìm phần tử trong cache theo IP
    HASH_FIND_STR(arp_cache, ip, entry);
    
    if (entry != NULL) {
        return entry->mac_addr;  // Trả về địa chỉ MAC nếu tìm thấy
    }
    return NULL;  // Nếu không tìm thấy, trả về NULL
}

// Hàm tra cứu phần tử trong cache
int lookup_element_to_cache(const char* ip) {
    struct arp_entry *entry;
    
    // Tìm phần tử trong cache theo IP
    HASH_FIND_STR(arp_cache, ip, entry);
    
    if (entry != NULL) {
        return 1;  // Nếu tìm thấy, trả về 1
    }
    return 0;  // Nếu không tìm thấy, trả về 0
}

// Hàm kiểm tra phần tử có tồn tại trong cache không
int is_element_exist(const char* ip) {
    struct arp_entry *entry;
    
    // Tìm phần tử trong cache theo IP
    HASH_FIND_STR(arp_cache, ip, entry);
    
    return entry != NULL;  // Trả về 1 nếu phần tử tồn tại, 0 nếu không
}

int main() {
    // Thêm vài phần tử vào cache
    add_to_cache("192.168.1.1", "00:11:22:33:44:55");
    add_to_cache("192.168.1.2", "00:11:22:33:44:56");
    
    // Kiểm tra tồn tại của phần tử
    if (is_element_exist("192.168.1.1")) {
        printf("IP 192.168.1.1 exists\n");
    }
    
    // Tra cứu phần tử trong cache
    char *mac = get_element_from_cache("192.168.1.1");
    if (mac != NULL) {
        printf("MAC address for 192.168.1.1: %s\n", mac);
    }
    
    // Kiểm tra phần tử
    int result = lookup_element_to_cache("192.168.1.2");
    if (result) {
        printf("Found IP 192.168.1.2\n");
    } else {
        printf("IP 192.168.1.2 not found\n");
    }
    
    return 0;
}
