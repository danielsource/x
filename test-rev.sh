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
	./x -r <test/x.hex >test/x.bin
	xxd -r <test/xxd.hex >test/xxd.bin
	diff test/xxd.bin test/x.bin
	i=$((i+1))
done

xxd <"$bigfile" >"$bigfile".hex

echo
echo "time x -r <$bigfile.hex"
time -p ./x -r <"$bigfile.hex" >test/x.bin

if [ -x ./rexxd ]; then
	echo
	echo "time rexxd -r <$bigfile.hex"
	time -p ./rexxd -r <"$bigfile.hex" >test/rexxd.bin
fi

echo
echo "time xxd -r <$bigfile.hex"
time -p xxd -r <"$bigfile.hex" >test/xxd.bin

echo
echo "test x == xxd with $bigfile.bin"
diff test/xxd.bin test/x.bin

echo
echo 'ok!'
