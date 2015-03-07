# set global variables
LIBS=-I./lib -I./src
CC=gcc
CFLAGS=-Wall

all: main
	$(CC) ./build/src/paging.o -o ./build/src/paging

main:
	$(CC) $(CFLAGS) ./src/paging.c -o ./build/src/paging.o

test:
	$(CC) $(CFLAGS) $(LIBS) ./src/paging.c ./lib/seatest.c ./test/paging_test.c -o ./build/test/paging_test

clean:
	rm ./build/test/*
	rm ./build/src/*


.PHONY: all test clean
