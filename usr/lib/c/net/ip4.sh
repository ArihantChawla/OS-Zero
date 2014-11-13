#! /bin/sh

gcc -DIP4TEST=1 -fno-builtin -g -Wall -DTESTRAND=0 -I.. -I../.. -I../../../.. -O -o ip4 ip4.c ../../zero/randmt32.c

