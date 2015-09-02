#! /bin/sh

#CC=gcc
CC=clang

#$CC -Wextra -D_REENTRANT=1 -DSHUNTC=1 -DNVALGRIND -DZERONEWMTX=0 -DZEROMALLOC=0 -DPTHREAD=1 -D_REENTRANT -g -Wall -O -I.. -I../m -fPIC -shared -o zerolib.so daemon.c data.c file.c futex.c hash.c maplk.c randk.c randlfg2.c randlfg.c randmt32.c randmt64.c unix.c util.c shuntc.c shunt.c -pthread
$CC -Wundef -Wextra -D_REENTRANT=1 -DSHUNTC=1 -DNVALGRIND -DZERONEWMTX=0 -DZEROMALLOC=0 -DPTHREAD=1 -D_REENTRANT -g -Wall -O -I.. -I../m -fPIC -shared -o zerolib.so *.c -pthread

