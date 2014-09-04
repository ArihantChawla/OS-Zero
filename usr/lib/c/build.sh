#! /bin/sh

CC=gcc

$CC -nostdinc -D_REENTRANT -g -Wall -O -I. -I.. -I../../.. -I../../../usr/lib -fPIC -nostdinc -fno-builtin -shared -o zlibc.so *.c -pthread

