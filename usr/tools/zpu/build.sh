#! /bin/sh

clang -DZEN=1 -DZPUPROF=1 -DZASBUF=0 -DZASMMAP=0 -DWPMVEC=0 -g -I.. -I../../lib -O0 -Wall -o zpu *.c

