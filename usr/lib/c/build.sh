#! /bin/sh

#CC=clang
#CCWARNFLAGS="-Wall -Wundef -Wextra -Wpointer-arith"
CC=gcc
CCBUILDFLAGS="-D_FILE_OFFSET_BITS=64 -D_LARGEFILE64_SOURCE=1 -D_ZERO_SOURCE=1 -DMEMLFHASH=1 -DMEMNEWHASH=1 -nostdlib"
CCWARNFLAGS="-D_FORTIFY_SOURCE=2 -Wall -Wundef -Wextra -Wpointer-arith -Wuninitialized -Winit-self -Wmissing-include-dirs -Wparentheses -Wswitch-default -Wswitch-bool -Wunused-parameter -Wstrict-aliasing -Wsuggest-attribute=pure -Wsuggest-attribute=const -Wsuggest-attribute=noreturn -Wsuggest-attribute=format -Wtrampolines -Wfloat-equal -Wshadow -Wstack-usage=16384 -Wunsafe-loop-optimizations -Wbad-function-cast -Wc99-c11-compat -Wcast-align -Wwrite-strings -Wdate-time -Wjump-misses-init -Wlogical-op -Waggregate-return -Wstrict-prototypes -Wnormalized -Wopenmp-simd -Wpacked -Wredundant-decls -Wnested-externs -Winline -Wvector-operation-performance -Wvla -Wstack-protector -Woverlength-strings -Wno-designated-init -Wdeclaration-after-statement -Wformat-security -Wmissing-format-attribute"

#$CC -Wundef -Wextra -DNVALGRIND -DZERONEWMTX=0 -DZEROMALLOC=0 -DPTHREAD=1 -D_REENTRANT -g -Wall -O -I. -I.. -I../m -I../../.. -I../../../usr/lib -fPIC -nostdinc -fno-builtin -ffreestanding -shared -o zlibc.so bsearch.c malloc2.c qsort.c rand.c setjmp.c string.c strings.c systime.c unistd.c bits/*.c sys/*.c sys/zero/*.c -pthread
#$CC -m32 -Wundef -Wextra -DNVALGRIND -DZERONEWMTX=0 -DZEROMALLOC=0 -DPTHREAD=1 -D_REENTRANT -g -Wall -O -I/usr/include -I. -I.. -I../m -I../../.. -I../../../usr/lib -fPIC -fno-builtin -ffreestanding -shared -o zlibc32.so *.c bits/*.c sys/*.c sys/zero/*.c ia32/*.S -pthread

#$CC $CCWARNFLAGS -DNVALGRIND -DZERONEWMTX=0 -DZEROMALLOC=0 -DPTHREAD=1 -D_REENTRANT -g -O -I/usr/include -I. -I.. -I../m -I../../.. -I../../../usr/lib -fPIC -fno-builtin -shared -o zlibc.so *.c bits/*.c sys/*.c sys/zero/*.c x86-64/*.S netinet/*.c -pthread
$CC $CCBUILDFLAGS $CCWARNFLAGS -msse -DNVALGRIND -DZERONEWMTX=0 -DZEROMALLOC=0 -D_REENTRANT -g -O -I. -I.. -I../m -I../../.. -I../../../usr/lib -fPIC -shared -fno-builtin -rdynamic -o zlibc.so *.c sys/*.c sys/zero/*.c x86-64/*.S ../zero/mem.c -pthread

#$CC -Wuninitialized -m32 -Wundef -Wextra -DNVALGRIND -DZERONEWMTX=0 -DZEROMALLOC=0 -DPTHREAD=1 -D_REENTRANT -g -Wall -O -I/usr/include -I. -I.. -I../m -I../../.. -I../../../usr/lib -fPIC -fno-builtin -shared -o zlibc32.so *.c bits/*.c sys/*.c sys/zero/*.c ia32/*.S -pthread

