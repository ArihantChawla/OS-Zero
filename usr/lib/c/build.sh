#! /bin/sh

CC=gcc

$CC -nostdinc -D_REENTRANT -g -Wall -O -I. -I.. -I../../../usr/lib -fPIC -shared -o zlibc.so *.c -pthread

