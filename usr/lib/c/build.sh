#! /bin/sh

CC=gcc

$CC -D_ZERO_SOURCE=1 -D_REENTRANT -g -Wall -O -I.. -I../../../usr/lib -fPIC -shared -o zmalloc.so malloc.c -pthread

