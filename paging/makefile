# set global variables
LIBS=-I./lib -I./src
CC=gcc
CFLAGS=-std=c99 -m32

all: test main

main: paging
	$(CC) $(CFLAGS) ./build/src/paging.o ./src/display.c -o ./build/src/display

paging:
	$(CC) $(CFLAGS) -c ./src/paging.c -o ./build/src/paging.o

test: paging
	$(CC) $(CFLAGS) $(LIBS) ./build/src/paging.o ./lib/seatest.c ./test/paging_test.c -o ./build/test/paging_test

clean:
	rm ./build/test/*
	rm ./build/src/*


.PHONY: all test clean
