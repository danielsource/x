This directory contains naive implementations in C of the following:

x - stupid simple hex dump (xxd(1) like)
usage: x [-i | -r]
example: x < a.out > a.hex
example: vim a.hex; x -r < a.hex > a.out
example: x -i < a.out > a.h

xs - search hex string
usage: xs HEXSTRING
example: xs deadbeef < file.bin  # outputs xxd-like offset if found


Build instructions
------------------
cc x.c -o x
cc xs.c -o xs

# or you can use the Makefile
