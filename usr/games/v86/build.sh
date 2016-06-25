#! /bin/sh

clang -Wall -Wextra -Wpointer-arith -I. -I.. -I../../lib -I../../.. -O -o v86 *.c asm/*.c cc/*.c
