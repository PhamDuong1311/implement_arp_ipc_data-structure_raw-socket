#ifndef CLI_H
#define CLI_H

#define MQ_NAME "/my_mq"

struct msgbuf {
    long mtype;
    char mtext[100];
};

void init_ipc_cli(const char *name);
void send_msg_to_daemon(const char *name);
void receive_msg_from_daemon(const char *name);
#endif
