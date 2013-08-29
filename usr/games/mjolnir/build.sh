#! /bin/sh

CC=clang

$CC -DTEST -Wall -g -I.. -I../../lib -o mjolnir *.c ../../lib/zero/randmt32.c -lncurses

