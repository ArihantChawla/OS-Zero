#! /bin/sh

CC=gcc
#CC=clang

$CC -DZCCSTAT=1 -DNEWHASH=1 -O -g -Wall -I../.. -I../../../../lib/ -I.. -o vcc cc.c cpp.c lex.c hash.c

