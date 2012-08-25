#! /bin/sh

#gcc -DZPCIMLIB2=1 -DZPCX11=1 -g -Wall -I.. -I../../lib -O0 -o zpc zpc.c stk.c cop.c zero.c expr.c x11.c imlib2.c -lX11 -lImlib2 -pthread
#gcc -DZPCIMLIB2=1 -DZPCX11=1 -g -Wall -I.. -I../../lib -O0 -o zpc zpc.c stk.c cop.c zero.c expr.c asm.c x11.c imlib2.c -lX11 -lImlib2 -pthread
gcc -DZPC=1 -DSMARTTYPES=1 -DZPCIMLIB2=1 -DZPCX11=1 -g -Wall -I.. -I../../lib -O0 -o zpc asm.c ../wpm/wasm.c calc.c stk.c cop.c zero.c expr.c vpu64.c x11.c imlib2.c -lX11 -lImlib2 -pthread
#gcc -DZPCIMLIB2=1 -DZPCX11=1 -g -Wall -I.. -I../../lib -O0 -o zpc zpc.c stk.c cop.c zero.c expr.c asm.c x11.c imlib2.c -lX11 -lImlib2 -pthread

