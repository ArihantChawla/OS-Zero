#! /bin/sh

gcc -DIP4TEST=1 -fno-builtin -g -Wall -DTESTRAND=0 -I.. -I../.. -I../../../usr/lib -O2 -o ip4 ../net/ip4.c ../../../usr/lib/zero/randmt32.c

