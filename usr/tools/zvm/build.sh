#! /bin/sh

clang -g -DZVMXCB=1 -DZASALIGN=1 -DZVMTRACE=1 -DZVMMULTITAB=0 -DZASNEWHACKS=1 -DZAS32BIT=1 -DZVM=1 -DZVMVIRTMEM=0 -O0 -I../../lib -I../.. -I.. -o zvm zvm.c mem.c op.c io.c asm.c sig.c xcb.c ../zas/zas.c ../zas/opt.c -lxcb

