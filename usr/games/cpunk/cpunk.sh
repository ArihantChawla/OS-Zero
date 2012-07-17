#! /bin/sh

gcc -D_ZERO_SOURCE=1 -g -Wall -I. -I.. -I../../lib -o cpunk character.c cpunk.c level.c spell.c ../dice.c ../../lib/zero/randk.c
