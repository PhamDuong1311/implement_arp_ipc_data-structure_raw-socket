#ifndef IPC_DAEMON_H
#define IPC_DAEMON_H

#define MQ_NAME "/my_mq"

extern char ip_target[256];

struct msgbuf {
    long mtype;
    char mtext[100];
};

void create_daemon(void);
void init_ipc_daemon(const char *name);
void send_msg_to_cli(const char *name, char *msg);
void receive_msg_from_cli(const char *name);

#endif
