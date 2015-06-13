#! /bin/sh

gcc -I../../lib -I.. -O0 -Wall -g -o cw cw.c rc.c
#gcc -DZEUSWINX11=1 -DZEUSIMLIB2=1 -DCWRANDMT32=1 -DZEUS=1 -I../../lib -I.. -O0 -Wall -g -o zeus cw.c rc.c zeus.c x11.c ../../lib/c/malloc.c ../../lib/zero/randmt32.c -lX11 -lImlib2 -pthread
gcc -DCOREWAR=1 -DZEUSWINX11=1 -DZEUSIMLIB2=1 -DCWRANDMT32=1 -DZEUS=1 -I../../lib -I.. -O0 -Wall -g -o zeus cw.c rc.c zeus.c x11.c ../../lib/zero/randmt32.c -lX11 -lImlib2 -pthread
#gcc -DZEUSWINX11=1 -DZEUSCLICKTOOLTIP=1 -DCWRANDMT32=1 -DZEUS=1 -I../../lib -I.. -O0 -Wall -g -o zeus cw.c rc.c zeus.c x11.c ../../lib/c/malloc.c ../../lib/zero/randmt32.c -lX11 -lImlib2 -pthread

