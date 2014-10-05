#! /bin/sh

CC=gcc

$CC -DTEST=0 -Wall -v -g -I.. -I../../lib -o mjolnir *.c ../../lib/zero/randmt32.c -lncurses

