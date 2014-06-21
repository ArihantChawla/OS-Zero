#! /bin/sh

clang -DZPUPROF=1 -g -I.. -I../../lib -O -Wall -o zpu *.c

