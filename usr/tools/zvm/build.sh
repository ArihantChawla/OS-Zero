#! /bin/sh

gcc -g -DZVMEFL=0 -DZVMXORG=0 -DZVMXCB=0 -DZASALIGN=1 -DZVMTRACE=1 -DZVMMULTITAB=0 -DZASNEWHACKS=1 -DZAS32BIT=1 -DZVM=1 -DZVMVIRTMEM=0 -O -I../../lib -I../.. -I.. -I/usr/include/freetype2 -o zvm zvm.c mem.c op.c io.c asm.c sig.c efl.c ../zas/zas.c ../zas/opt.c -lX11 -lImlib2 -lfreetype
gcc -g -DZVMEFL=1 -DZVMXORG=0 -DZVMXCB=0 -DZASALIGN=1 -DZVMTRACE=1 -DZVMMULTITAB=0 -DZASNEWHACKS=1 -DZAS32BIT=1 -DZVM=1 -DZVMVIRTMEM=0 -O -I../../lib -I../.. -I.. -I/usr/include/freetype2 -o ezvm zvm.c mem.c op.c io.c asm.c sig.c efl.c ../zas/zas.c ../zas/opt.c `pkg-config ecore ecore-evas --cflags --libs`

