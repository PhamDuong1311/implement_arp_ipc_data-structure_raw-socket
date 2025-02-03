// Đã test

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <mqueue.h>
#include <string.h>
#include <fcntl.h>
#include "ipc_daemon.h"

char ip_target[256] = "";


void create_daemon(void) {
    pid_t pid = fork();

    if (pid < 0) {
        perror("Fork failed");
        exit(EXIT_FAILURE);
    }

    if (pid > 0) {
        exit(EXIT_SUCCESS);  
    }

    if (setsid() < 0) {
        perror("setsid failed");
        exit(EXIT_FAILURE);
    }

    if (chdir("/") < 0) {
        perror("chdir failed");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < 3; i++) {
        close(i);  
    }

    printf("Daemon created\n");
}

void init_ipc_daemon(const char *name) {
    mqd_t mq = mq_open(name, O_CREAT | O_RDWR, 0666, NULL);
    if (mq == (mqd_t) -1) {
        perror("mq_open failed");
        exit(EXIT_FAILURE);
    }
    printf("Message Queue created\n");

    mq_close(mq);
}

void send_msg_to_cli(const char *name, char* msg) {
    mqd_t mq = mq_open(name, O_WRONLY);
    if (mq == (mqd_t) -1) {
        perror("mq_open failed");
        exit(EXIT_FAILURE);
    }

    struct msgbuf message;
    message.mtype = 1;  
    strcpy(message.mtext, msg);

    if (mq_send(mq, (const char*)&message, sizeof(message.mtext), 0) == -1) {
        perror("mq_send failed");
        exit(EXIT_FAILURE);
    }

    printf("MAC address target sent to CLI: %s\n", message.mtext);

    mq_close(mq);
}

void receive_msg_from_cli(const char *name) {
    mqd_t mq = mq_open(MQ_NAME, O_RDONLY);
    if (mq == (mqd_t) -1) {
        perror("mq_open failed");
        exit(EXIT_FAILURE);
    }

    struct mq_attr attr;
    if (mq_getattr(mq, &attr) == -1) {
        perror("mq_getattr failed");
        exit(EXIT_FAILURE);
    }

    struct msgbuf message;
    ssize_t bytes_read = mq_receive(mq, (char*)&message, attr.mq_msgsize, NULL); 
    if (bytes_read == -1) {
        perror("mq_receive failed");
        exit(EXIT_FAILURE);
    }
    
    strncpy(ip_target, message.mtext, sizeof(ip_target) - 1);
    ip_target[sizeof(ip_target) - 1] = '\0'; 

    printf("IP address received from CLI: %s\n", message.mtext);

    mq_close(mq);
}
