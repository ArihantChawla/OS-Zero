#! /bin/sh

gcc -DTRANSLATE=0 -g -Wall -I.. -I../../lib -O -o rex rex.c
