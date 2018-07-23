#! /bin/sh

echo "building mtx-tutorial examples"
gcc -I.. -o mtx-tutorial mtx-tutorial.c 2> build.log

