#include "module_daemon/arp_cache.h"
#include "module_daemon/arp.h"
#include "module_daemon/ipc_daemon.h"
#include "module_daemon/timeout.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main() {
    create_daemon();
    init_ipc_daemon(MQ_NAME);
    
    while (1) {
        receive_msg_from_cli(MQ_NAME);
        
        struct interface_info src_info = get_src_addr("wlan0");
        send_arp_request("wlan0", target_ip);
        receive_arp_reply();

        // Lưu vào cache
        lookup_element_to_cache(target_ip, "AA:BB:CC:DD:EE:FF");
        
        // Kiểm tra cache và gửi phản hồi
        char *mac_addr = get_element_from_cache(target_ip);
        if (mac_addr) {
            send_msg_to_cli(MQ_NAME, mac_addr);
        }

        // Xóa cache quá hạn
        remove_element_expired(arp_cache_size, arp_cache);
        
        sleep(5); // Lặp lại sau mỗi 5 giây
    }
    
    return 0;
}
