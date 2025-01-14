#ifndef IPC_DAEMON_H
#define IPC_DAEMON_H

void create_daemon(void);
void init_ipc_daemon();
void send_msg_to_cli();
void receive_msg_from_cli();

#endif
