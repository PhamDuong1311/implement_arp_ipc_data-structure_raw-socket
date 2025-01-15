#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>

#include "module_daemon/arp_cache.h"
#include "module_daemon/arp.h"
#include "module_daemon/ipc_daemon.h"
#include "module_daemon/timeout.h"

#define MSG_QUEUE_NAME "/my_message_queue"  


int main() {
    const char *iface = "eth0";

    // Lấy thông tin địa chỉ IP và MAC của interface
    struct interface_info info = get_src_addr(iface);

    printf("Interface: %s\n", iface);
    printf("IP Address: %d.%d.%d.%d\n",
           info.ip_addr[0], info.ip_addr[1], info.ip_addr[2], info.ip_addr[3]);
    printf("MAC Address: %02x:%02x:%02x:%02x:%02x:%02x\n",
           info.mac_address[0], info.mac_address[1], info.mac_address[2],
           info.mac_address[3], info.mac_address[4], info.mac_address[5]);

    // Gửi ARP request
    const char *target_ip = "192.168.1.1";
    send_arp_request(iface, target_ip);

    // Nhận ARP reply
    receive_arp_reply();

    // Khởi tạo daemon IPC
    create_daemon();
    init_ipc_daemon();

    return 0;
}
