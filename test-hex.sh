#!/bin/sh

set -e

bigfile=64M
dd if=/dev/urandom of="$bigfile" bs=1M count=64

i=0; while [ $i -lt 1024 ]; do
	dd if="$bigfile" bs=1 count=$i 2>/dev/null | ./x >x.hex
	dd if="$bigfile" bs=1 count=$i 2>/dev/null | xxd >xxd.hex
	diff xxd.hex x.hex
	i=$((i+1))
done

echo
echo "time x <"$bigfile""
time -p x <"$bigfile" >x.hex

echo
echo "time xxd <"$bigfile""
time -p xxd <"$bigfile" >xxd.hex

echo
diff xxd.hex x.hex

echo 'ok!'
