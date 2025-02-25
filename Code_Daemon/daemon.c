/*
- Thread 1 (Xử lý yêu cầu từ CLI):
    + Xử lý các yêu cầu nhanh (show, delete, add, etc...)
    + Đẩy yêu cầu search MAC vào queue 1
- Thread 2 (Xử lý arp từ CLI):
    + Đọc queue 1
    + Kiểm tra có cache miss không, có thì yêu cầu ARP req (có retry và timeout) và blocking
    + Chờ ARP rep (xử lý cả ARP req, rep cùng 1 thread vì chỉ có single CLI)
- Thread 3 (Xử lý ARP req từ Device)
    + Nếu kernel không có cơ chế từ DROP packet ko phù hợp thì tự code
    + Sau khi DROP, các ARP req phù hơp cho vào queue 2
- Thread 4 (Gửi ARP reply cho Device)
    + Đọc queue 2
    + Gửi ARP rep cho đúng Device
- Thread 5 (Xử lý ARP cache)
    + Timeout 15s
*/



#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include "module_daemon/arp_cache.h"
#include "module_daemon/ipc_daemon.h"
#include "module_daemon/timeout.h"

pthread_mutex_t cache_mutex = PTHREAD_MUTEX_INITIALIZER;

void *arp_cache_manager(void *arg) {
    while (1) {
        sleep(15);
        pthread_mutex_lock(&cache_mutex);
        remove_element_expired(arp_cache_size, arp_cache);
        pthread_mutex_unlock(&cache_mutex);
    }
    return NULL;
}

void *main_worker(void *arg) {
     //create_daemon();
    int server_sock = setup_socket();
    printf("Daemon is running...\n");

    while (1) {
        receive_request(server_sock);
        pthread_mutex_unlock(&cache_mutex);
    }

    unlink(SOCKET_PATH);
    return NULL;
}

int main() {
    pthread_t thread_main, thread_cache;

    pthread_create(&thread_main, NULL, main_worker, NULL);
    pthread_create(&thread_cache, NULL, arp_cache_manager, NULL);

    pthread_join(thread_main, NULL);
    pthread_join(thread_cache, NULL);

    return 0;
}
