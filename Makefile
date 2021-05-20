SOURCE_DIR = src
EXAMPLE_DIR = examples
INCLUDE_DIR = include
OBJECT_DIR = obj
SOURCE_FILES = $(wildcard $(SOURCE_DIR)/*.c)
HEADERS = $(wildcard $(INCLUDE_DIR)/*.h)
OBJECTS = $(patsubst $(SOURCE_DIR)/%.c, $(OBJECT_DIR)/%.o, $(SOURCE_FILES))
CFLAGS = -g -pthread -Wall -Wextra

.PHONY: default run clean

default: main

main: $(PWD)/libhttpserver.so $(OBJECT_DIR)/main.o
	gcc $(CFLAGS) $^ -o $@ -lsqlite3 -ljson-c

$(PWD)/libhttpserver.so: $(OBJECTS)
	gcc $(CFLAGS) $^ -shared -fPIC -o $@

$(OBJECT_DIR)/main.o : $(EXAMPLE_DIR)/main.c $(HEADERS) | $(OBJECT_DIR)
	gcc $(CFLAGS) -Iinclude -I/usr/include/json-c -c $< -o $@

$(OBJECT_DIR)/%.o : $(SOURCE_DIR)/%.c $(HEADERS) | $(OBJECT_DIR)
	gcc $(CFLAGS) -fPIC -Iinclude -I/usr/include/json-c -c $< -o $@

$(OBJECT_DIR) :
	mkdir -p $@

run: main
	./main

clean:
	$(RM) main $(PWD)/libhttpserver.so $(OBJECT_DIR)/main.o $(OBJECTS)
