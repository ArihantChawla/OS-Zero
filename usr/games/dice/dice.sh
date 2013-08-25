#! /bin/sh

gcc -fno-builtin -DDICE_SRAND_TIME=1 -DTEST=1 -I../lib -o dice dice.c ../lib/zero/randk.c

