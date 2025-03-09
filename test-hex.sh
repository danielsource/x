#!/bin/sh

set -e

for f in readme.txt x.c testfiles-hex/* x /bin/sh; do
	echo "test $f"
	./x < "$f" > x.hex
	xxd < "$f" > xxd.hex
	diff xxd.hex x.hex
	cat x.hex
	echo
done

rm x.hex xxd.hex
echo 'ok!'
