#! /bin/sh

CC=gcc

$CC -g -Wall -I.. -I../../lib -o zdb *.c ../../lib/ui/*.c ../../lib/ui/sys/*.c -lX11

