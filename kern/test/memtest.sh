gcc -O0 -g -Wall -D__KERNEL__=0 -DSMP=0 -DMEMTEST=1 -DMEMPRINT=1 -I../.. -I../../usr/lib -o memtest memtest.c ../mem/*.c -pthread
