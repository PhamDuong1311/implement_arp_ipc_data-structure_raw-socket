CC = gcc
CFLAGS = -Wall -pthread -lrt

SRC = pending_queue.c main_thread.c arp.c arp_cache.c ipc_daemon.c
OBJ = $(SRC:.c=.o)
TARGET = arp

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)

.PHONY: all clean
