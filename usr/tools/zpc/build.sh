#! /bin/sh

#gcc -DZPCIMLIB2=1 -DZPCX11=1 -g -Wall -I.. -I../../lib -O0 -o zpc zpc.c stk.c cop.c zero.c expr.c x11.c imlib2.c -lX11 -lImlib2 -pthread
gcc -DZPCIMLIB2=1 -DZPCX11=1 -g -Wall -I.. -I../../lib -I../../lib/c -O0 -o zpc zpc.c stk.c cop.c zero.c expr.c x11.c imlib2.c -lX11 -lImlib2 -pthread

