#! /bin/sh

#CC=gcc
CC=gcc

#$CC -Wundef -Wextra -DNVALGRIND -DZERONEWMTX=0 -DZEROMALLOC=0 -DPTHREAD=1 -D_REENTRANT -g -Wall -O -I. -I.. -I../m -I../../.. -I../../../usr/lib -fPIC -nostdinc -fno-builtin -ffreestanding -shared -o zlibc.so bsearch.c malloc2.c qsort.c rand.c setjmp.c string.c strings.c systime.c unistd.c bits/*.c sys/*.c sys/zero/*.c -pthread
$CC -m32 -Wundef -Wextra -DNVALGRIND -DZERONEWMTX=0 -DZEROMALLOC=0 -DPTHREAD=1 -D_REENTRANT -g -Wall -O -I/usr/include -I. -I.. -I../m -I../../.. -I../../../usr/lib -fPIC -fno-builtin -ffreestanding -shared -o zlibc32.so *.c bits/*.c sys/*.c sys/zero/*.c ia32/*.S -pthread

$CC -Wundef -Wextra -DNVALGRIND -DZERONEWMTX=0 -DZEROMALLOC=0 -DPTHREAD=1 -D_REENTRANT -g -Wall -O -I/usr/include -I. -I.. -I../m -I../../.. -I../../../usr/lib -fPIC -fno-builtin -ffreestanding -shared -o zlibc.so *.c bits/*.c sys/*.c sys/zero/*.c -pthread

