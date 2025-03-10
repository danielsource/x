CC = gcc
CFLAGS = -std=c89 -O2 -g3 -Wall -Wpedantic

all: x xs

clean:
	rm -f x xs

x: x.c
	$(CC) $(CFLAGS) -o x x.c

xs: xs.c
	$(CC) $(CFLAGS) -o xs xs.c

# https://nullprogram.com/blog/2025/02/17/
rexxd: rexxd.c
	gcc -std=gnu11 -O2 -o rexxd rexxd.c
