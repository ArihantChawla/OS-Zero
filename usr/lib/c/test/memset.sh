#! /bin/sh

CC=clang

$CC -DMEMSETDUALBANK=1 -I.. -I../.. -I../../../.. -g -O -Wall -fno-builtin -o memset2b memset.c ../string.c
$CC -DMEMSETDUALBANK=0 -I.. -I../.. -I../../../.. -g -O -Wall -fno-builtin -o memset memset.c ../string.c

