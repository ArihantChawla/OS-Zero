#! /bin/sh

gcc -g -O0 -Wall -I.. -o lkpool lkpool.c ../zero/qrand.c ../zero/hash.c -pthread

