SOURCE_PATH = $(CURDIR)/src

default: src/main.c
	gcc $(SOURCE_PATH)/*.c -o server

run:
	./server

clean:
	rm server