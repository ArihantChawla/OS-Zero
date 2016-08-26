#! /bin/sh

gcc -g -Wall -O -o mandel1 mandel1.c -lX11 -pthread
gcc -g -Wall -O -o mandel2 mandel2.c -lX11 -pthread
gcc -msse -g -Wall -O -o mandel3 mandel3.c mandel_sse.S -lX11 -pthread
gcc -msse -m128bit-long-double -g -Wall -O -o mandel4 mandel4.c -lX11 -pthread
gcc -msse2 -g -Wall -O -o mandel5 mandel5.c mandel_sse.S -lX11 -pthread
#gcc -I../../lib -D_REENTRANT -msse -m128bit-long-double -g -Wall -O -o mandelthr mandelthr.c mandel_sse.S -lX11 -pthread
gcc -I../../lib -D_REENTRANT -msse -m128bit-long-double -g -Wall -O -o mandelthr mandelthr.c -lX11 -pthread
