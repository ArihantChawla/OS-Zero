#! /bin/bash

clang -DVTTEST=1 -I.. -g -Wall -O -o test *.c ../ui/ui.c ../ui/sys/xorg.c -lX11

