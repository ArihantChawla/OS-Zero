#! /bin/sh

CC=gcc
#CC=clang

# As of October 24 2011, gcc -O2 produces slower code than -O // vendu
# - same thing with -finline-functions and -funroll-loops
$CC -DMALLOCMULTITAB=1 -DZEROHASH=1 -DZEROFUTEX=0 -DZEROMALLOC=1 -DPTHREAD=1 -D_REENTRANT -g -Wall -fno-builtin -O -I. -o heap heap.c -pthread
$CC -DX11VIS=0 -DMALLOCMULTITAB=1 -DZEROHASH=1 -DZEROFUTEX=0 -DZEROMALLOC=1 -DPTHREAD=1 -pg -D_REENTRANT -g -Wall -fno-builtin -O0 -I../../../.. -I../../../lib -o zheap heap.c ../malloc.c -pthread
$CC -DMALLOCMULTITAB=1 -DZEROHASH=1 -DZEROFUTEX=0 -DZEROMALLOC=1 -DPTHREAD=1 -D_REENTRANT -DX11VIS=1 -g -Wall -fno-builtin -O -I../../../.. -I../../../lib -o xzheap heap.c ../malloc.c -Wl,-lX11 -pthread
#$CC -DPTHREAD=1 -DX11VIS=1 -D_REENTRANT -g -Wall -fno-builtin -O -I../../../.. -I../../../lib /../include -o xheap heap.c ../stdlib/../malloc.c -pthread -lX11
#$CC -DPTHREAD=1 -D_REENTRANT -g -Wall -fno-builtin -O -I../../../.. -I../../../lib /../include -o jeheap heap.c libjemalloc.a -pthread -ldl
$CC -DX11VIS=0 -DMALLOCMULTITAB=1 -DZEROHASH=1 -DZEROFUTEX=0 -DZEROMALLOC=1 -DPTHREAD=1 -D_REENTRANT -DDEVEL=0 -g -Wall -fno-builtin -O -I../../../.. -I../../../lib -fPIC -shared -o zmalloc.so ../malloc.c -pthread
$CC -DMALLOCMULTITAB=1 -DZEROHASH=1 -DZEROFUTEX=0 -DZEROMALLOC=1 -DPTHREAD=1 -D_REENTRANT -DX11VIS=1 -DEVEL=0 -g -Wall -fno-builtin -O -I../../../.. -I../../../lib -fPIC -shared -o xzmalloc.so ../malloc.c -lX11 -pthread
#$CC -DPTHREAD=1 -DX11VIS=1 -D_REENTRANT -g -Wall -fno-builtin -O -I../../../.. -I../../../lib /../include -fPIC -shared -o xmalloc.so ../stdlib/../malloc.c -pthread

