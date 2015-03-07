# set global variables
CC=gcc
CFLAGS=-c -Wall

all: paging
	$(CC) ./build/paging.o -o ./build/paging

paging:
	$(CC) $(CFLAGS) ./src/paging.c -o ./build/paging.o

clean:
	rm ./build/*
