#! /bin/sh
gcc -g -mmmx -Wall -O0 -DUSE_SHM=0 -g -I../.. -I/usr/local/include -I/usr/X11R6/include -o testx11 testx11.c error.c scale.c scale2x.c -L/usr/local/lib -L/usr/X11R6/lib -lImlib2 -lXcomposite -lXv -lX11 -lXext

