This directory contains _naive_ implementations in C of the following:

x - stupid simple hex dump (xxd(1) like)
usage: x [-i|-r|-v] [< FILE]
example: x < a.out > a.hex
example: vim a.hex; x -r < a.hex > a.out
example: x -i < a.out > a.c

xs - search hex string
usage: xs HEX_OCTETS [< FILE]
       xs -v
example: xs deadbeef < file.bin  # outputs xxd-like offset if found


Build instructions
------------------
cc x.c -o x         # or you can use the Makefile
cc xs.c -o xs


./x <x | head       # just an example
00000000: 7f45 4c46 0201 0100 0000 0000 0000 0000  .ELF............
00000010: 0300 3e00 0100 0000 d010 0000 0000 0000  ..>.............
00000020: 4000 0000 0000 0000 58b2 0000 0000 0000  @.......X.......
00000030: 0000 0000 4000 3800 0d00 4000 2600 2500  ....@.8...@.&.%.
00000040: 0600 0000 0400 0000 4000 0000 0000 0000  ........@.......
00000050: 4000 0000 0000 0000 4000 0000 0000 0000  @.......@.......
00000060: d802 0000 0000 0000 d802 0000 0000 0000  ................
00000070: 0800 0000 0000 0000 0300 0000 0400 0000  ................
00000080: 1803 0000 0000 0000 1803 0000 0000 0000  ................
00000090: 1803 0000 0000 0000 1c00 0000 0000 0000  ................
