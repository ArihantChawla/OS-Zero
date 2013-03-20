#! /bin/sh

gcc -g -O -Wall -I.. -I../.. -I../../lib -I../../usr/lib/c -o y2k y2k.c ../dice.c ../../lib/c/rand.c
