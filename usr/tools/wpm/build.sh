#! /bin/sh

CC=gcc

#gcc -DWPMDB=1 -I.. -I../../lib -O -g -Wall -o asm asm.c wpm.c -pthread
#$CC -DWPMTRACE=1 -I.. -I../../lib -O -g -Wall -o asm asm.c wpm.c mem.c -pthread
#gcc -DASMPREPROC=1 -DWPMTRACE=1 -I.. -I../../lib -O0 -g -Wall -o asm asm.c wpm.c mem.c -pthread
gcc -DASMPREPROC=1 -I.. -I../../lib -O0 -g -Wall -o asm asm.c wpm.c mem.c -pthread

