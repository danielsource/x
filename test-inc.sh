#!/bin/sh

set -e

for f in testfiles-hex/* ; do
	f=$(basename "$f")
	echo "test testfiles-inc/${f}.h"
	./x -i < testfiles-hex/"$f" | diff testfiles-inc/"$f".h -
	cat testfiles-inc/"$f".h
	echo
done

echo 'ok!'
