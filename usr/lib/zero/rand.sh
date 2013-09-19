#! /bin/sh

gcc -I.. -O -o rand randmt32.c randk.c randlfg.c randlfg2.c test.c

