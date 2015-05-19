#! /bin/sh

gcc -g -O0 -I.. -I../../lib -o cell test.c cell.c ../../lib/zero/randmt32.c

