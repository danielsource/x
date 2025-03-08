CC = gcc
CFLAGS = -std=c89 -O0 -g3 -fanalyzer -Wall -Wpedantic

all: x

clean:
	rm -f x xs

x: x.c
	$(CC) $(CFLAGS) -o x x.c

xs: xs.c
	$(CC) $(CFLAGS) -o xs xs.c
