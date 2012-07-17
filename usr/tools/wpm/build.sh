#! /bin/sh

CC=clang

#gcc -DWPMDB=1 -I.. -I../../lib -O -g -Wall -o asm asm.c wpm.c -pthread
$CC -DWPMTRACE=1 -I.. -I../../lib -O -g -Wall -o asm asm.c wpm.c mem.c -pthread
#gcc -I.. -I../../lib -O -g -Wall -o asm asm.c wpm.c mem.c -pthread

