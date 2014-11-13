#! /bin/sh

CC=gcc
#CC=clang

# As of October 24 2011, gcc -O2 produces slower code than -O // vendu
# - same thing with -finline-functions and -funroll-loops
$CC -DZEROFUTEX=0 -DZEROMALLOC=1 -DPTHREAD=1 -D_REENTRANT -g -Wall -O -I. -o heap heap.c -pthread
$CC -DZEROFUTEX=0 -DZEROMALLOC=1 -DPTHREAD=1 -pg -D_ZERO_SOURCE=1 -D_REENTRANT -g -Wall -O0 -I../.. -o zheap heap.c ../malloc.c ../../zero/futex.c -pthread
$CC -DZEROFUTEX=0 -DZEROMALLOC=1 -DPTHREAD=1 -D_ZERO_SOURCE=1 -D_REENTRANT -DX11VIS=1 -g -Wall -O -I../.. -o xzheap heap.c ../malloc.c ../../zero/futex.c -Wl,-lX11 -pthread
#$CC -DPTHREAD=1  -DX11VIS=1 -D_ZERO_SOURCE=1 -D_REENTRANT -g -Wall -O -I../../../include -o xheap heap.c ../stdlib/../malloc.c ../../zero/futex.c -pthread -lX11
#$CC -DPTHREAD=1  -D_REENTRANT -g -Wall -O -I../../../include -o jeheap heap.c libjemalloc.a -pthread -ldl
$CC -DZEROFUTEX=0 -DZEROMALLOC=1 -DPTHREAD=1 -D_ZERO_SOURCE=1 -D_REENTRANT -DDEVEL=0 -g -Wall -O -I../.. -fPIC -shared -o zmalloc.so ../malloc.c ../../zero/futex.c -pthread
$CC -DZEROFUTEX=0 -DZEROMALLOC=1 -DPTHREAD=1 -D_ZERO_SOURCE=1 -D_REENTRANT -DX11VIS=1 -DEVEL=0 -g -Wall -O -I../.. -fPIC -shared -o xzmalloc.so ../malloc.c ../../zero/futex.c -lX11 -pthread
#$CC -DPTHREAD=1  -DX11VIS=1 -D_ZERO_SOURCE=1 -D_REENTRANT -g -Wall -O -I../../../include -fPIC -shared -o xmalloc.so ../stdlib/../malloc.c ../../zero/futex.c -pthread

