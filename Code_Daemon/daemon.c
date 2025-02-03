#include "module_daemon/arp_cache.h"
#include "module_daemon/ipc_daemon.h"
#include "module_daemon/arp.h"
#include "module_daemon/timeout.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main() {
    create_daemon();
    init_ipc_daemon(MQ_NAME);
    lookup_element_to_cache("192.168.1.1", "00:11:22:33:44:55"); // test

    while (1) {
        remove_element_expired(arp_cache_size, arp_cache);
        receive_msg_from_cli(MQ_NAME);
        printf("Read from extern var: %s\n", ip_target);

        char *mac = get_element_from_cache(ip_target);
        if (mac != NULL) {
            printf("MAC was found: %s\n", mac);
            send_msg_to_cli(MQ_NAME, mac);

        }

        else {
            send_arp_request("wlan0", ip_target);
            receive_arp_reply("wlan1");
            lookup_element_to_cache(ip_target, mac_target);
            send_msg_to_cli(MQ_NAME, mac_target);
        }

    }
    
    return 0;
}
