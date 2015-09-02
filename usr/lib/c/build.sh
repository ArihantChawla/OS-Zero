#! /bin/sh

#CC=gcc
CC=clang

$CC -Wextra -DNVALGRIND=1 -DZERONEWMTX=0 -DZEROMALLOC=0 -DPTHREAD=1 -D_REENTRANT -g -Wall -O -I. -I.. -I../m -I../../.. -I../../../usr/lib -fPIC -nostdinc -fno-builtin -ffreestanding -shared -o zlibc.so bsearch.c malloc2.c qsort.c rand.c setjmp.c string.c strings.c systime.c unistd.c bits/*.c sys/*.c sys/zero/*.c -pthread
#$CC -DZERONEWMTX=1 -DZEROMALLOC=0 -DPTHREAD=1 -D_REENTRANT -g -Wall -O -I. -I.. -I../m -I../../.. -I../../../usr/lib -fPIC -fno-builtin -shared -o zlibc.so bsearch.c malloc2.c qsort.c rand.c setjmp.c string.c strings.c systime.c unistd.c sys/*.c sys/zero/*.c -pthread

