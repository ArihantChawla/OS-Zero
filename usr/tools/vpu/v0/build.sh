#! /bin/sh

gcc -Wall -I.. -I../../../lib -I../.. -O -o vm io.c vm.c ../../../lib/zero/fastudiv.c

