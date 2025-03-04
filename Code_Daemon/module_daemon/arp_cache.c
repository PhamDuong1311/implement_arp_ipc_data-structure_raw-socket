#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
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
    struct arp_entry *entry, *tmp;
    char entry_str[150];
    char time_str[30];    
    
    snprintf(response, response_size, "ARP Table:\n");

    HASH_ITER(hh, arp_cache_head, entry, tmp) {
        struct tm *tm_info = localtime(&entry->timestamp);
        strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", tm_info);

        snprintf(entry_str, sizeof(entry_str), "%s -> %02X:%02X:%02X:%02X:%02X:%02X | Last seen: %s\n",
                 entry->ip_addr,
                 entry->mac_addr[0], entry->mac_addr[1], entry->mac_addr[2],
                 entry->mac_addr[3], entry->mac_addr[4], entry->mac_addr[5],
                 time_str);

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

void remove_element_expired() {
    struct arp_entry *entry, *tmp;
    time_t current_time = time(NULL);

    HASH_ITER(hh, arp_cache_head, entry, tmp) {
        if ((current_time - entry->timestamp) > 2) { 
            HASH_DEL(arp_cache_head, entry); 
            arp_cache_size--;  
            printf("Đã xóa ARP entry hết hạn: %s\n", entry->ip_addr);
        }
    }
}


int is_element_expired(time_t timestamp) {
    time_t current_time = time(NULL);
    
    if ((current_time - timestamp) > 100) return 1;
    return 0; 
}

// int main() {
//     unsigned char mac1[6] = {0x11, 0x11, 0x11, 0x11, 0x11, 0x11};
//     unsigned char mac2[6] = {0x22, 0x22, 0x22, 0x22, 0x22, 0x22};

//     lookup_element_to_cache("111.111.111.111", mac1);
//     lookup_element_to_cache("222.222.222.222", mac2);

//     char response[1000];
//     show_arp_cache(response, sizeof(response));
//     printf("%s\n", response);

//     printf("Chờ 5 giây để mục hết hạn...\n");
//     sleep(2);

//     remove_element_expired();
    
//     show_arp_cache(response, sizeof(response));
//     printf("%s\n", response);
//     return 0;
// }
