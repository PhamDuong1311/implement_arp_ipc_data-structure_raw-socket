#include "module_daemon/arp_cache.h"
#include "module_daemon/ipc_daemon.h"
#include "module_daemon/arp.h"
#include "module_daemon/timeout.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main() {
    // create_daemon();
    init_ipc_daemon(MQ_NAME);

    //test mac
    unsigned char mac[] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66};  
    lookup_element_to_cache("192.168.1.1", mac);

    while (1) {
        remove_element_expired(arp_cache_size, arp_cache);
        receive_msg_from_cli(MQ_NAME);
        printf("Read from extern var: %s\n", ip_target);

        unsigned char *mac = get_element_from_cache(ip_target);

        if (mac != NULL) {
            printf("MAC was found: %02X:%02X:%02X:%02X:%02X:%02X\n",
                   mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

            char mac_str[18];
            sprintf(mac_str, "%02X:%02X:%02X:%02X:%02X:%02X",
                    mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

            // Gửi MAC đến client
            send_msg_to_cli(MQ_NAME, mac_str);
        }
        else {
/*          send_arp_request("wlan0", ip_target);
            receive_arp_reply("wlan1");
            lookup_element_to_cache(ip_target, mac_target);
            send_msg_to_cli(MQ_NAME, (char *)mac_target); */

        }

    }
    
    return 0;
}
