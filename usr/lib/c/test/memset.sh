#! /bin/sh

CC=gcc

$CC -I.. -I../.. -I../../../.. -g -O -Wall -fno-builtin -o memset memset.c

