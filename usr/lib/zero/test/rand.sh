#! /bin/sh

gcc -fno-builtin -DRANDMT32TEST=1 -DRANDMT32PROF=1 -g -I.. -I../.. -O -o rand ../randmt32.c ../randk.c ../randlfg.c ../randlfg2.c
gcc -g -O -o test32 mt19937ar.c
gcc -g -O -o test64 mt19937-64.c
gcc -DRANDMT32TEST=1 -g -Wall -O0 -I../.. -o randmt32 ../randmt32.c
gcc -DRANDMT64TEST=1 -g -Wall -O0 -I../.. -o randmt64 ../randmt64.c

