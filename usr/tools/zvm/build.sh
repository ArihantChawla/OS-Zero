#! /bin/sh

clang -g -DZAS32BIT=1 -DZVM=1 -DZVMVIRTMEM=0 -O -I../../lib -I../.. -I.. -o zvm zvm.c op.c asm.c ../zas/zas.c

