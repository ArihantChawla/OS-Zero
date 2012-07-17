#! /bin/sh

gcc -D_GNU_SOURCE=1 -DMATHTEST=1 -D_ZERO_SOURCE=1 -D_REENTRANT -g -Wall -O -I. -I../../../usr/lib -o mathtest i387/math.c -lm
