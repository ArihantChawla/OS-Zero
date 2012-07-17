#! /bin/sh

gcc -Wall -g -DBUFTEST=1 -I. -I.. -I../kern -I../usr/lib -o buftest buftest.c buf.c

