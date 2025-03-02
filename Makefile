CC = gcc
CFLAGS = -Wall -Iinclude
LDFLAGS = -lncurses
INSTALL_DIR ?= /usr/local/bin

SRC = $(wildcard src/*.c)
OBJ = $(SRC:.c=.o)
EXEC = bin/hibou

all: $(EXEC)

bin:
	mkdir -p bin

$(EXEC): $(OBJ)
	$(CC) $(OBJ) -o $(EXEC) $(LDFLAGS)

%.o: %.c | bin
	$(CC) $(CFLAGS) -c $< -o $@

install: $(EXEC)
	sudo cp $(EXEC) $(INSTALL_DIR)/
	sudo chmod +x $(INSTALL_DIR)/hibou

clean:
	rm -f $(OBJ) $(EXEC)

.PHONY: all clean bin install
