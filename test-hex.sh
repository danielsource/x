#!/bin/sh

set -e

mkdir -p test

bigfile=test/64MiB
if [ ! -e "$bigfile" ]; then
	dd if=/dev/urandom of="$bigfile" bs=1M count=64 2>/dev/null
fi

echo "test x == xxd up to 512 bytes"
i=0; while [ $i -le 512 ]; do
	dd if="$bigfile" bs=1 count=$i 2>/dev/null | ./x >test/x.hex
	dd if="$bigfile" bs=1 count=$i 2>/dev/null | xxd >test/xxd.hex
	diff test/xxd.hex test/x.hex
	i=$((i+1))
done

echo
echo "time x <$bigfile"
time -p ./x <"$bigfile" >test/x.hex

if [ -x ./rexxd ]; then
	echo
	echo "time rexxd <$bigfile"
	time -p ./rexxd <"$bigfile" >test/rexxd.hex
fi

echo
echo "time xxd <$bigfile"
time -p xxd <"$bigfile" >test/xxd.hex

echo
echo "test x == xxd with $bigfile"
diff test/xxd.hex test/x.hex

echo
echo 'ok!'
