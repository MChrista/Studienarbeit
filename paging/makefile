# set global variables
LIBS=-I./lib -I./src
CC=gcc
CFLAGS=-Wall -std=c99

all: main
	$(CC) ./build/src/display.o -o ./build/src/paging

main:
	$(CC) $(CFLAGS) ./src/display.c -o ./build/src/display.o

test:
	$(CC) $(CFLAGS) $(LIBS) ./src/paging.c ./src/display.c ./lib/seatest.c ./test/paging_test.c -o ./build/test/paging_test

clean:
	rm ./build/test/*
	rm ./build/src/*


.PHONY: all test clean
