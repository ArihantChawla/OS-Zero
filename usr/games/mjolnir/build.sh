#! /bin/sh

CC=clang

$CC -Wall -g -I.. -I../../lib -o mjolnir *.c -lncurses

