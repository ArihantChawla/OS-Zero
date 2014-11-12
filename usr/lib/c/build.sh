#! /bin/sh

#CC=gcc
CC=clang

#$CC -DPTHREAD=0 -D_REENTRANT -g -Wall -O -I. -I.. -I../../.. -I../../../usr/lib -fPIC -nostdinc -fno-builtin -shared -o zlibc.so *.c -pthread
#$CC -DPTHREAD=0 -DMTSAFE=1 -D_REENTRANT -g -Wall -O -I. -I.. -I../../.. -I../../../usr/lib -fPIC -nostdinc -fno-builtin -shared -o zlibc.so *.c sys/zero/*.c -pthread
$CC -DZEROMALLOC=0 -DPTHREAD=1 -D_REENTRANT -g -Wall -O -I. -I.. -I../../.. -I../../../usr/lib -fPIC -nostdinc -fno-builtin -ffreestanding -shared -o zlibc.so *.c sys/zero/*.c -pthread

