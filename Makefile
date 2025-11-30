CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -Iinclude

SRC = 	src/main.c \
		src/fs.c \
		src/init/fs_init.c \
		src/shell/fs_shell_parser.c \
		src/helpers/fs_helpers.c \
		src/helpers/fcb_helpers.c \
		src/cmd/menu.c \
		src/cmd/commands.c \
		src/helpers/permissions.c \

		
OBJ = $(SRC:.c=.o)
BIN = mini_fs

all: $(BIN)

$(BIN): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -f $(OBJ) $(BIN)
