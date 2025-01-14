CC = gcc
CFLAGS = -Wall -g
SRC = main.c module_daemon/arp.c module_daemon/arp_cache.c module_daemon/ipc_daemon.c module_daemon/timeout.c
OBJ = $(SRC:.c=.o)
TARGET = daemon

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -f $(OBJ) $(TARGET)
