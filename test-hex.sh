#!/bin/sh

set -e

for f in readme.txt x.c testfiles-hex/* /bin/sh; do
	echo "test $f"
	./x < "$f" > x.hex
	xxd < "$f" > xxd.hex
	diff xxd.hex x.hex
	cat x.hex
	echo
done

echo 'ok!'
