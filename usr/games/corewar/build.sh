#! /bin/sh

gcc -Wextra -Wundef -I../../lib -I.. -O -Wall -Wextra -Wundef -Wpointer-arith -g -o cw cw.c rc.c ../../lib/zero/randmt32.c -pthread
gcc -DZEUS=1 -Wextra -Wundef -I../../lib -I.. -O -Wall -Wextra -Wundef -Wpointer-arith -g -o zeus cw.c rc.c zeus.c x11.c ../../lib/zero/randmt32.c -lX11 -lImlib2 -pthread

