CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -Iinclude

SRC = src/main.c src/fs.c
OBJ = $(SRC:.c=.o)
BIN = mini_fs

all: $(BIN)

$(BIN): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -f $(OBJ) $(BIN)
