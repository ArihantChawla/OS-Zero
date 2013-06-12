#! /bin/sh

clang -g -Wall -O0 -mmmx -msse2 -I.. -o test test.c
