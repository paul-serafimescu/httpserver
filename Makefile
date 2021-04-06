SOURCE_PATH = $(CURDIR)/src
SOURCE_FILES = $(wildcard $(SOURCE_PATH)/*.c)
CFLAGS = -Wall -Wextra -Iinclude

.PHONY: default run clean

default: server

server: $(SOURCE_FILES)
	gcc $(CFLAGS) $^ -o $@

run: server
	./server

clean:
	$(RM) server
