SOURCE_DIR = src
OBJECT_DIR = obj
SOURCE_FILES = $(wildcard $(SOURCE_DIR)/*.c)
OBJECTS = $(patsubst $(SOURCE_DIR)/%.c, $(OBJECT_DIR)/%.o, $(SOURCE_FILES))
CFLAGS = -g -pthread -lsqlite3 -Wall -Wextra -Iinclude

.PHONY: default run clean

default: server

server: $(OBJECTS)
	gcc $(CFLAGS) $^ -o $@

$(OBJECT_DIR)/%.o : $(SOURCE_DIR)/%.c | $(OBJECT_DIR)
	gcc $(CFLAGS) -c $< -o $@

$(OBJECT_DIR) :
	mkdir -p $@

run: server
	./server

clean:
	$(RM) server $(OBJECTS)
