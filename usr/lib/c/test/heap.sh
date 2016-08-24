#! /bin/sh

#CC=clang
#CCWARNFLAGS="-Wall -Wundef -Wextra -Wpointer-arith"
CC=gcc
CCWARNFLAGS="-D_FORTIFY_SOURCE=2 -Wall -Wundef -Wextra -Wpointer-arith -Wuninitialized -Winit-self -Wmissing-include-dirs -Wparentheses -Wswitch-default -Wswitch-bool -Wunused-parameter -Wstrict-aliasing -Wsuggest-attribute=pure -Wsuggest-attribute=const -Wsuggest-attribute=noreturn -Wsuggest-attribute=format -Wtrampolines -Wfloat-equal -Wshadow -Wstack-usage=16384 -Wunsafe-loop-optimizations -Wbad-function-cast -Wc99-c11-compat -Wcast-align -Wwrite-strings -Wdate-time -Wjump-misses-init -Wlogical-op -Waggregate-return -Wstrict-prototypes -Wnormalized -Wopenmp-simd -Wpacked -Wredundant-decls -Wnested-externs -Winline -Wvector-operation-performance -Wvla -Wstack-protector -Woverlength-strings -Wno-designated-init -Wdeclaration-after-statement -Wformat-security -Wmissing-format-attribute"

# As of October 24 2011, gcc -O2 produces slower code than -O // vendu
# - same thing with -finline-functions and -funroll-loops
$CC $CCWARNFLAGS -msse2 -DZEROHASH=0 -DZEROFUTEX=0 -DZEROMALLOC=1 -DPTHREAD=1 -D_REENTRANT -g -Wall -fno-builtin -O -fno-omit-frame-pointer -I../.. -o heap heap.c -pthread
$CC $CCWARNFLAGS -msse2 -DZEROHASH=0 -DZEROFUTEX=0 -DZEROMALLOC=1 -DPTHREAD=1 -D_REENTRANT -g -Wall -fno-builtin -O -fno-omit-frame-pointer -I../.. -o llheap lockless_allocator/ll_alloc.c heap.c -pthread
$CC $CCWARNFLAGS -msse2 -DZEROHASH=0 -DZEROFUTEX=0 -DZEROMALLOC=1 -DPTHREAD=1 -D_REENTRANT -g -Wall -fno-builtin -O -fno-omit-frame-pointer -I../.. -o tcheap heap.c -Wl,-L/usr/local/lib -ltcmalloc -pthread
$CC $CCWARNFLAGS -msse2 -DZEROHASH=0 -DZEROFUTEX=0 -DZEROMALLOC=1 -DPTHREAD=1 -D_REENTRANT -g -Wall -fno-builtin -O -fno-omit-frame-pointer -I../.. -o jeheap heap.c -Wl,-L/usr/local/lib -ljemalloc -pthread
$CC $CCWARNFLAGS -msse2 -DX11VIS=0 -DZEROHASH=0 -DZEROFUTEX=0 -DZEROMALLOC=1 -DPTHREAD=1 -D_REENTRANT -g -Wall -fno-builtin -DZEROTHR=0 -O -fno-omit-frame-pointer -I../.. -I../../../.. -I../../../lib -o zheap ../_string.c heap.c ../../zero/hash64.c ../../zero/priolk.c ../../zero/lfdeq.c ../../zero/hash32.c  ../malloc.c -pthread
#$CC $CCWARNFLAGS -msse2 -DX11VIS=0 -DMALLOCTRACE=1 -DZEROHASH=0 -DZEROFUTEX=0 -DZEROMALLOC=1 -DPTHREAD=1 -D_REENTRANT -g -Wall -fno-builtin -O -fno-omit-frame-pointer -I../../../.. -I../../../lib -o tzheap heap.c ../execinfo.c ../../zero/priolk.c ../../zero/lfdeq.c ../../zero/hash32.c  ../malloc.c -pthread
$CC $CCWARNFLAGS -msse2 -DZEROHASH=0 -DZEROFUTEX=0 -DZEROMALLOC=1 -DPTHREAD=1 -D_REENTRANT -DX11VIS=1 -g -Wall -fno-builtin -O -fno-omit-frame-pointer -I../.. -I../../../.. -I../../../lib -o xzheap ../_string.c heap.c ../../zero/hash64.c ../../zero/priolk.c ../../zero/lfdeq.c ../../zero/hash32.c  ../malloc.c -Wl,-lX11 -pthread
$CC $CCWARNFLAGS -msse2 -DZEROHASH=0 -DZEROFUTEX=0 -DZEROMALLOC=1 -DPTHREAD=1 -D_REENTRANT -DX11VIS=1 -g -Wall -fno-builtin -O -fno-omit-frame-pointer -I../.. -I../../../.. -I../../../lib -o t-test1 ../_string.c ../../zero/hash64.c ../../zero/priolk.c ../../zero/lfdeq.c ../../zero/hash32.c  ../malloc.c t-test1.c -Wl,-lX11 -pthread
$CC $CCWARNFLAGS -msse2 -DZEROHASH=0 -DZEROFUTEX=0 -DZEROMALLOC=1 -DPTHREAD=1 -D_REENTRANT -DX11VIS=1 -g -Wall -fno-builtin -O -fno-omit-frame-pointer -I../.. -I../../../.. -I../../../lib -o tmem ../_string.c ../../zero/hash64.c ../../zero/priolk.c ../../zero/lfdeq.c ../../zero/hash32.c  ../malloc.c tmem.c -Wl,-lX11 -pthread
#$CC $CCWARNFLAGS -msse2 -DPTHREAD=1 -DX11VIS=1 -D_REENTRANT -g -Wall -fno-builtin -O -fno-omit-frame-pointer -I../../../.. -I../../../lib /../include -o xheap heap.c ../stdlib/../../zero/priolk.c ../../zero/lfdeq.c ../../zero/hash32.c  ../malloc.c -pthread -lX11
#$CC $CCWARNFLAGS -msse2 -DPTHREAD=1 -D_REENTRANT -g -Wall -fno-builtin -O -fno-omit-frame-pointer -I../../../.. -I../../../lib /../include -o jeheap heap.c libjemalloc.a -pthread -ldl
$CC $CCWARNFLAGS -msse2 -DX11VIS=0 -DZEROHASH=0 -DZEROFUTEX=0 -DZEROMALLOC=1 -DPTHREAD=1 -D_REENTRANT -DDEVEL=0 -g -Wall -fno-builtin -shared -fPIC -O -fno-omit-frame-pointer -I../../../.. -I../../../lib -fPIC -shared -o zmalloc.so -I../.. ../_string.c ../../zero/hash64.c ../../zero/priolk.c ../../zero/lfdeq.c ../../zero/hash32.c ../malloc.c -pthread

