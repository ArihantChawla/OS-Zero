#! /bin/sh

CC=gcc

$CC -Wall -O0 -DMJOLDEBUG=1 -DTEST=0 -Wall -g -I.. -I../../lib -o mjolnir *.c ../../lib/zero/randmt32.c -lncurses

