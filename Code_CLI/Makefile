CC = gcc
CFLAGS = -Wall -pthread -lrt

SRC = cli.c
OBJ = $(SRC:.c=.o)
EXEC = cli

all: $(EXEC)

$(EXEC): $(OBJ)
	$(CC) -o $(EXEC) $(OBJ) $(CFLAGS)


%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(EXEC)

.PHONY: all clean
