CC = gcc
CFLAGS = -Wall -pthread -lrt

SRC = daemon.c module_daemon/arp_cache.c module_daemon/ipc_daemon.c module_daemon/timeout.c
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
