# set global variables
LIBS=-I./lib -I./src
CC=gcc
CFLAGS=-m32 -Wall -Werror -Wextra -g -O2 -std=gnu99

all: test

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
