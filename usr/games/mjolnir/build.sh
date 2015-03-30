#! /bin/sh

CC=clang

$CC -Wall -O -DMJOLDEBUG=1 -DTEST=0 -Wall -v -g -I.. -I../../lib -o mjolnir *.c ../../lib/zero/randmt32.c -lncurses

