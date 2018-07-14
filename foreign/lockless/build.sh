#! /bin/sh

CC=gcc

$CC -I../../usr/lib -g -Wall -O -ffast-math -o mandel mandel.c -lX11
$CC -I../../usr/lib -g -Wall -O -ffast-math -o mandel1 mandel1.c -lX11 -pthread
$CC -I../../usr/lib -g -Wall -O -ffast-math -o mandel2 mandel2.c -lX11 -pthread
$CC -msse -m128bit-long-double -I../../usr/lib -g -Wall -O -ffast-math -o mandel3 mandel3.c mandel_sse.S -lX11 -pthread
$CC -msse -m128bit-long-double -I../../usr/lib -g -Wall -O -ffast-math -o mandel4 mandel4.c -lX11 -pthread
#$CC -msse2 -I../../usr/lib -g -Wall -O -ffast-math -o mandel5 mandel5.c mandel_sse.S -lX11 -pthread
#$CC -I../../lib -D_REENTRANT -msse -m128bit-long-double -I../../usr/lib -g -Wall -O -ffast-math -o mandelthr mandelthr.c mandel_sse.S -lX11 -pthread
$CC -Wundef -Wpointer-arith -g -I../../lib -D_REENTRANT -msse -I../../usr/lib -g -Wall -O3 -o mandelthr mandelthr.c mandel_sse.S -lX11 -lm -pthread
#$CC -I../../lib -D_REENTRANT -I../../usr/lib -g -Wall -O -ffast-math -o mandelthr mandelthr.c -lX11 -pthread
