gcc -O -g -Wall -D__KERNEL__=0 -DSMP=1 -DMTSAFE=1 -DMAGSLABLK=1 -DMAGLK=0 -DMAGBITMAP=0 -DMEMTEST=1 -I../.. -I../../usr/lib -o memtest memtest.c ../mem/mag.c ../mem/slab.c -pthread
