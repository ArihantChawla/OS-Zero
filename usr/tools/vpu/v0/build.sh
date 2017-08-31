#! /bin/sh

gcc -Wall -I.. -I../../../lib -I../.. -O -o vm *.c ../../../lib/zero/fastudiv.c ../../../lib/zero/hash.c ../../../tools/zas/*.c

