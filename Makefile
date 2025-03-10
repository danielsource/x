CC = gcc
CFLAGS = -std=c89 -O2 -g3 -Wall -Wpedantic

all: x

clean:
	rm -f x xs

x: x.c
	$(CC) $(CFLAGS) -o x x.c

xs: xs.c
	$(CC) $(CFLAGS) -o xs xs.c
