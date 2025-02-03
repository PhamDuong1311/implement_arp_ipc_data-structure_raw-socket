// Đã test

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mqueue.h>
#include <unistd.h>
#include "cli.h"

void init_ipc_cli(const char *name) {
    mqd_t mq = mq_open(name, O_CREAT | O_RDWR, 0666, NULL);
    if (mq == (mqd_t) -1) {
        perror("mq_open failed");
        exit(EXIT_FAILURE);
    }
    printf("Message Queue initialized\n");

    mq_close(mq);
}

void send_msg_to_daemon(const char *name) {
    char msg[256];
    printf("Enter IP target: ");
    fgets(msg, sizeof(msg), stdin);
    msg[strcspn(msg, "\n")] = '\0';

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

    printf("IP address target sent to daemon: %s\n", message.mtext);

    mq_close(mq);
}

void receive_msg_from_daemon(const char *name) {
    mqd_t mq = mq_open(name, O_RDONLY);
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

    printf("MAC address target received from daemon: %s\n", message.mtext);

    mq_close(mq);
}

int main() {
    init_ipc_cli(MQ_NAME);
    send_msg_to_daemon(MQ_NAME);

    receive_msg_from_daemon(MQ_NAME);
    return 0;
}