#! /bin/sh

gcc -Wall -g -I. -I.. -I../kern -I../usr/lib -o btest bitmap.c ../kern/bitmap.c

