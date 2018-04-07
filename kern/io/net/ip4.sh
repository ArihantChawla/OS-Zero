#! /bin/sh

#gcc -D_REENTRANT -DIP4TEST=1 -fno-builtin -g -Wall -DTESTRAND=0 -O -I../../../u
gcc -DIP4TEST=1 -fno-builtin -g -Wall -DTESTRAND=0 -O -I../../../usr/lib -o ip4 ../../../usr/lib/zero/ip4.c ../../../usr/lib/zero/randmt32.c

