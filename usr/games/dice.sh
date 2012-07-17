#! /bin/sh

gcc -fno-builtin -DTEST=1 -I../lib -o dice dice.c ../lib/zero/randk.c

