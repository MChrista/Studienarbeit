# set global variables
LIBS=-I./lib -I./src
CC=gcc
CFLAGS=-m32 -Wall -Werror -Wextra -g -O2 -std=gnu99

all: pagingtests

paging:
	$(CC) $(CFLAGS) -c ../pfhandler.c -o ./pfhandler.o

test: paging
	$(CC) $(CFLAGS) $(LIBS) ./build/src/paging.o ./lib/seatest.c ./test/paging_test.c -o ./build/test/paging_test

pagingtests: paging
	$(CC) $(CFLAGS) ./pfhandler.o ./paging_test.c -o ../paging_test

clean:
	rm ../paging_test
	rm ./pfhandler.o


.PHONY: all test clean
