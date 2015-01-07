#! /bin/sh

CC=gcc

$CC -DX11VIS=0 -DMALLOCMULTITAB=1 -DZEROHASH=1 -DZEROFUTEX=0 -DZEROMALLOC=1 -DPTHREAD=1 -D_REENTRANT -g -Wall -fno-builtin -O0 -I../../../.. -I../../../lib -o zheap2 heap.c ../malloc2.c ../../zero/hash.c -pthread
$CC -DX11VIS=0 -DMALLOCMULTITAB=1 -DZEROHASH=1 -DZEROFUTEX=0 -DZEROMALLOC=1 -DPTHREAD=1 -D_REENTRANT -DDEVEL=0 -g -Wall -fno-builtin -O -I../../../.. -I../../../lib -fPIC -shared -o zmalloc2.so ../malloc2.c ../../zero/hash.c -pthread


