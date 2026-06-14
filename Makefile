CC=gcc
CFLAGS=-Wall -Wextra -pedantic -std=c99 \
       -D_DEFAULT_SOURCE \
       -D_BSD_SOURCE \
       -D_GNU_SOURCE
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

# Installation
PREFIX ?= /usr/local
BINDIR ?= $(PREFIX)/bin

install: release
	mkdir -p $(DESTDIR)$(BINDIR)
	install -m 755 $(BIN) $(DESTDIR)$(BINDIR)/$(BIN)
	@echo "✓ Installed blink to $(DESTDIR)$(BINDIR)/$(BIN)"

uninstall:
	rm -f $(DESTDIR)$(BINDIR)/$(BIN)
	@echo "✓ Uninstalled blink"

clean:
	rm -rf $(OBJ_DIR) $(BIN)