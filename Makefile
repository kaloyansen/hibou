BINARY_DIR = bin
SOURCE_DIR = src
INCLUDE_DIR = include
INSTALL_DIR ?= /usr/local/bin

BINARY = hibou
TARGET = $(BINARY_DIR)/$(BINARY)

CC = gcc
CFLAGS = -Wall -I$(INCLUDE_DIR)
LDFLAGS = -lncurses

SRC = $(wildcard $(SOURCE_DIR)/*.c)
HDR = $(wildcard $(INCLUDE_DIR)/*.h)
OBJ = $(patsubst $(SOURCE_DIR)/%.c, $(BINARY_DIR)/%.o, $(SRC))

all: $(TARGET)
	@echo $<

$(BINARY_DIR):
	@mkdir -p $@

$(TARGET): $(BINARY_DIR) $(OBJ)
	@echo link $? ... $@
	$(CC) $(OBJ) $(LDFLAGS) -o $@

$(BINARY_DIR)/%.o: $(SOURCE_DIR)/%.c $(INCLUDE_DIR)/%.h
	@echo compile $< ... $@
	$(CC) $(CFLAGS) -c $< -o $@

install: $(TARGET)
	sudo cp $< $(INSTALL_DIR)/.
	sudo chmod +x $(INSTALL_DIR)/$(BINARY)

clean:
	rm -f $(OBJ)

clobber: clean
	rm -f $(TARGET)

.PHONY: all clean clobber install
