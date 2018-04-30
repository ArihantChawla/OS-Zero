#! /bin/sh

gcc -DVASDEBUG=1 -g -Wall -I.. -I../../../lib -I../.. -O -o vm *.c ../../../lib/zero/fastudiv.c ../../../lib/zero/hash.c ../vas/*.c

