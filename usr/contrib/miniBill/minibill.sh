#! /bin/sh

gcc -O -o def minibill.c -lm -lncurses -pthread
gcc -O -DMINIBILLSINE=1 -o sine minibill.c -lm -lncurses -pthread
gcc -O -DMINIBILLSAW=1 -o saw minibill.c -lm -lncurses -pthread
gcc -O -DMINIBILLSQUARE=1 -o square minibill.c -lm -lncurses -pthread
gcc -O -DMINIBILLFANCY1=1 -o fancy1 minibill.c -lm -lncurses -pthread
gcc -O -DMINIBILLFANCY2=1 -o fancy2 minibill.c -lm -lncurses -pthread

