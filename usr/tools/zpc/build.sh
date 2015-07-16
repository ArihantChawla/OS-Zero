#! /bin/sh

#gcc -DZPCIMLIB2=1 -DZPCX11=1 -g -Wall -I.. -I../../lib -O0 -o zpc zpc.c stk.c cop.c zero.c expr.c x11.c imlib2.c -lX11 -lImlib2 -pthread
#gcc -DZPCIMLIB2=1 -DZPCX11=1 -g -Wall -I.. -I../../lib -O0 -o zpc zpc.c stk.c cop.c zero.c expr.c asm.c x11.c imlib2.c -lX11 -lImlib2 -pthread
#gcc -DZPC=1 -DSMARTTYPES=1 -DZPCIMLIB2=1 -DZPCX11=1 -g -Wall -I.. -I../../lib -O0 -o zpc asm.c ../zas/zas.c ../wpm/mem.c zpc.c stk.c alu.c zero.c expr.c vpu64.c x11.c imlib2.c -lX11 -lImlib2 -pthread
clang -DSHUNTC=1 -DSHUNTZPC=1 -DZVM=1 -DZPCDEBUG=1 -DZAS32BIT=1 -DWPMWORDSIZE=32 -DZPC=0 -DSMARTTYPES=0 -DSMARTRADIX=1 -DZPCIMLIB2=1 -DZPCX11=1 -g -Wall -I.. -I../tools -I../../lib -O -o zpc ../zvm/zvm.c ../zvm/asm.c ../zvm/op.c ../zvm/mem.c ../zvm/io.c ../zvm/sig.c ../zas/zas.c ../zas/opt.c ../zas/io.c ../wpm/mem.c ../x11/x11.c zpc.c stk.c alu.c zero.c expr.c vpu64.c x11.c imlib2.c ../../lib/zero/hash.c ../../lib/zero/shunt.c -lX11 -lImlib2 -pthread
#gcc -DZPCIMLIB2=1 -DZPCX11=1 -g -Wall -I.. -I../../lib -O0 -o zpc zpc.c stk.c cop.c zero.c expr.c asm.c x11.c imlib2.c -lX11 -lImlib2 -pthread

