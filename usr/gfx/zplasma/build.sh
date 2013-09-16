#! /bin/sh

CC=gcc

$CC -I../../lib -DPROF=0 -O -Wall -o plasma plasma.c -lm -lSDL
$CC -I../../lib -DTEST=1 -DZPLASMA=1 -O -Wall -o zplasma zplasma.c -lm -lSDL



