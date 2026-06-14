CC=gcc
CFLAGS=-Wall -Wextra -pedantic -std=c99
SRC_DIR=src
OBJ_DIR=obj
BIN=blink

SRCS=$(wildcard $(SRC_DIR)/*.c)
OBJS=$(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRCS))

.PHONY: all clean debug release install uninstall

all: release

debug: CFLAGS += -g -O0 -DDEBUG
debug: $(BIN)

release: CFLAGS += -O3
release: $(BIN)

$(BIN): $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

# Installation paths and rules
PREFIX ?= /usr/local
BINDIR ?= $(PREFIX)/bin

install: release
	mkdir -p $(DESTDIR)$(BINDIR)
	cp -f $(BIN) $(DESTDIR)$(BINDIR)/$(BIN)
	chmod 755 $(DESTDIR)$(BINDIR)/$(BIN)

uninstall:
	rm -f $(DESTDIR)$(BINDIR)/$(BIN)

clean:
	rm -rf $(OBJ_DIR) $(BIN)

