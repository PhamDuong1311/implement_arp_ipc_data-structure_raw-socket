#ifndef GENERAL_H
#define GENERAL_H

#include <time.h>
#include <stdint.h>

#include "uthash.h"

#define MAX_ARP_CACHE_SIZE 100
#define SOCKET_PATH "/tmp/arp_socket"
#define QUEUE_SIZE 10  

// Định nghĩa cấu trúc 1 entry trong ARP cache
struct arp_entry {
    char ip_addr[16];      
    uint8_t mac_addr[6];     
    time_t timestamp;      
    UT_hash_handle hh;     
}; 

// Định nghĩa cấu trúc header của gói ARP
struct arp_header {
    uint16_t htype;
    uint16_t ptype;
    uint8_t hlen;
    uint8_t plen;
    uint16_t oper;
    uint8_t sha[6];  
    uint8_t spa[4];  
    uint8_t tha[6];  
    uint8_t tpa[4]; 
};

// Định nghĩa cấu trúc 1 phần tử trong Pending Queue
typedef struct {
    char ip[16];      
    unsigned char mac[6]; 
    time_t timestamp;        
    int count2;             
    int status;              
} queue_item_t;

// Định nghĩa cấu trúc Pending queue
typedef struct {
    queue_item_t items[QUEUE_SIZE]; 
    int front, rear, size;       
} queue_t;

typedef struct {
    char cmd;
    char ip[16];
    char mac[18];
} msg_t;

typedef enum {
    ARP_NONE = 0,    
    ARP_REQUEST,     
    ARP_REPLY        
} arp_status_t;

extern int arp_cache_size;
extern struct arp_entry *arp_cache_head;
extern struct arp_entry arp_cache[MAX_ARP_CACHE_SIZE];

extern uint8_t mac_dst[6];
extern uint8_t ip_dst[4];
extern uint8_t mac_src[6];
extern uint8_t ip_src[4];

extern char ip_str[16];
extern int exist_mac;

extern queue_t pending_queue;


#endif
