#! /bin/sh

gcc -I.. -I../../lib -O -o zed *.c ../../lib/zero/unix.c ../../lib/zero/file.c

