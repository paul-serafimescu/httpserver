SOURCE_DIR = src
EXAMPLE_DIR = examples
INCLUDE_DIR = include
OBJECT_DIR = obj
SOURCE_FILES = $(wildcard $(SOURCE_DIR)/*.c)
HEADERS = $(wildcard $(INCLUDE_DIR)/*.h)
OBJECTS = $(patsubst $(SOURCE_DIR)/%.c, $(OBJECT_DIR)/%.o, $(SOURCE_FILES))
CFLAGS = -g -pthread -Wall -Wextra
IFLAGS = -Iinclude -I/usr/include/json-c
LDFLAGS = -lsqlite3 -ljson-c

.PHONY: default run clean

default: main

main: $(OBJECT_DIR)/main.o $(PWD)/libhttpserver.so
	gcc $(CFLAGS) $^ -o $@ $(LDFLAGS)

$(PWD)/libhttpserver.so: $(OBJECTS)
	gcc $(CFLAGS) $^ -shared -fPIC -o $@ $(LDFLAGS)

$(OBJECT_DIR)/main.o : $(EXAMPLE_DIR)/main.c $(HEADERS) | $(OBJECT_DIR)
	gcc $(CFLAGS) $(IFLAGS) -c $< -o $@

$(OBJECT_DIR)/%.o : $(SOURCE_DIR)/%.c $(HEADERS) | $(OBJECT_DIR)
	gcc $(CFLAGS) -fPIC $(IFLAGS) -c $< -o $@

$(OBJECT_DIR) :
	mkdir -p $@

run: main
	./main

clean:
	$(RM) main $(PWD)/libhttpserver.so $(OBJECT_DIR)/main.o $(OBJECTS)
