#! /bin/sh

clang -DTESTSYM=1 -I../../lib -O -Wall -o testsym sym.c
clang -DZPUDEBUG=1 -DZEN=1 -DZPUPROF=1 -DZASBUF=0 -DZASMMAP=0 -DWPMVEC=0 -g -I.. -I../../lib -O0 -Wall -o zpu *.c

