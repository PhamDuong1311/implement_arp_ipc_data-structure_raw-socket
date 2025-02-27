CC = gcc
CFLAGS = -Wall -pthread -lrt

SRC = daemon.c module_daemon/pending_queue.c module_daemon/main_thread.c module_daemon/arp.c module_daemon/arp_cache.c module_daemon/ipc_daemon.c
OBJ = $(SRC:.c=.o)
TARGET = daemon

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)

.PHONY: all clean
