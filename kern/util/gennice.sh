#! /bin/sh

gcc -O -o gennice gennice.c
./gennice > ../nice.h

