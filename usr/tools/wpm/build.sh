#! /bin/sh

CC=gcc

#gcc -DWPMDB=1 -I.. -I../../lib -O -g -Wall -o asm asm.c wpm.c -pthread
#$CC -DWPMTRACE=1 -I.. -I../../lib -O -g -Wall -o asm asm.c wpm.c mem.c -pthread
#gcc -DASMPREPROC=1 -DWPMTRACE=1 -I.. -I../../lib -O0 -g -Wall -o asm asm.c wpm.c mem.c -pthread
#$CC -I.. -I../../lib -O -g -Wall -o wpm ../zas/zas.c expr.c wpm.c mem.c -pthread
$CC -DWPMVC=1 -DZAS32BIT=1 -DZASBUF=0 -DZASMMAP=1 -DZASPROF=1 -DWPM=1 -I.. -I../../lib -O -g -Wall -o wpm ../zas/zas.c expr.c wpm.c mem.c ../vcode/vc.c ../vcode/op.c -pthread

