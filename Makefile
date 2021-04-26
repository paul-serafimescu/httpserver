SOURCE_PATH = $(CURDIR)/src
SOURCE_FILES = $(wildcard $(SOURCE_PATH)/*.c)
CFLAGS = -g -pthread -Wall -Wextra -Iinclude

.PHONY: default run clean

default: server

server: $(SOURCE_FILES)
	gcc $(CFLAGS) $^ -o $@ -lsqlite3

run: server
	./server

clean:
	$(RM) server
