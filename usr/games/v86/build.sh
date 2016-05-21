#! /bin/sh

gcc -Wall -Wextra -Wpointer-arith -I. -I.. -I../../lib -O -o v86 *.c asm/*.c cc/*.c
