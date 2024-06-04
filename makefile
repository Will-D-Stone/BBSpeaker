CC=gcc
CFLAGS=-I.

test.o: LAB3.c
    $(CC) -c -o test.o BBSpeaker.c

test: test.o
    $(CC) -o test test.o

.PHONY: clean
clean:
    rm -f test.o test