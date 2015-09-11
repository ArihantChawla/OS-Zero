#! /bin/sh

gcc -g -Wall -O -DPTHREAD=1 -DTEST=1 -I.. -I../.. -I../../../.. -o sysconf ../unistd.c ../../zero/x86/cpuid.c -pthread

