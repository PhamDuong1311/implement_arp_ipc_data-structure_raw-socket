#include "module_daemon/arp_cache.h"
#include "module_daemon/ipc_daemon.h"
#include "module_daemon/timeout.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main() {
    //create_daemon();

    int server_sock = setup_socket();
    printf("Daemon is running...\n");

    receive_request(server_sock);
    unlink(SOCKET_PATH);
    
    return 0;
}
