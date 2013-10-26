gcc -O -g -Wall -D__KERNEL__=0 -DSMP=0 -DEVPROF=1 -DEVTEST=1 -I../.. -I../../usr/lib -o evtest evtest.c ../ev.c -pthread
