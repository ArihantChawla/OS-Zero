#! /bin/sh

CC=clang

$CC -g -O -I.. -I../../lib -I/usr/include/freetype2 -o zfc ../x11/x11.c zfc.c -lX11 -lImlib2 -lfreetype

