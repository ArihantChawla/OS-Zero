#! /bin/sh

#CC=gcc
CC=clang

#$CC -D_REENTRANT -g -Wall -O -I. -I.. -I../../.. -I../../../usr/lib -fPIC -nostdinc -fno-builtin -shared -o zlibc.so *.c -pthread
$CC -Wextra -D_REENTRANT -g -Wall -O -I. -I.. -I../../.. -I../../../usr/lib -fPIC -nostdinc -fno-builtin -shared -o zlibc.so bsearch.c qsort.c setjmp.c string.c strings.c -pthread

