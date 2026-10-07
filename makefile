CC = gcc

SRC_FOLDER = src/
SRC_OBJS = drag.o viewporter-protocol.o \
		wlr-layer-shell-unstable-v1-protocol.o \
		xdg-shell-client-protocol.o
OBJS = $(addprefix build/,$(SRC_OBJS))
DEBUG_OBJS = $(addprefix build/debug/,$(SRC_OBJS))
DEBUG_CFLAGS ?= -g -O0 -DDEBUG

LIBS = $(shell pkgconf --libs wayland-client wayland-cursor)

.PHONY: all debug clean

all: $(OBJS)
	$(CC) $^ -Iinclude $(LIBS) -o drag

debug: $(DEBUG_OBJS)
	$(CC) $^ -Iinclude $(LIBS) -o drag

build/%.o: $(SRC_FOLDER)%.c
	@mkdir -p build
	$(CC) $(CFLAGS) -Iinclude -c $< -o $@

build/debug/%.o: $(SRC_FOLDER)%.c
	@mkdir -p build/debug
	$(CC) $(CFLAGS) $(DEBUG_CFLAGS) -Iinclude -c $< -o $@

clean:
	rm -f *.o $(OBJS) $(DEBUG_OBJS)
	rm -r build
	rm drag
