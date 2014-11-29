#! /bin/sh

gcc -g -DZVMEFL=0 -DZVMTRACE=1 -DZVMMULTITAB=0 -DZASNEWHACKS=1 -O -I../../lib -I../.. -I.. -I/usr/include/freetype2 -o zvm zvm.c mem.c op.c io.c asm.c sig.c efl.c ../zas/zas.c ../zas/opt.c ../zas/io.c ../../lib/zero/hash.c -lX11 -lImlib2 -lfreetype
gcc -g -DZASZEROHASH=1 -DZVMEFL=1 -DZVMTRACE=1 -DZVMMULTITAB=0 -DZASNEWHACKS=1 -DZVMVIRTMEM=0 -O -I../../lib -I../.. -I.. -I/usr/include/freetype2 -o ezvm zvm.c mem.c op.c io.c asm.c sig.c efl.c ../zas/zas.c ../zas/opt.c ../zas/io.c ../../lib/zero/hash.c `pkg-config ecore ecore-evas edje evas --cflags --libs`

