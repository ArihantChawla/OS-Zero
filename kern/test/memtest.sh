gcc -O0 -fno-builtin -g -Wall -DMEMTEST=1 -I../.. -I../../usr/lib -o memtest memtest.c ../mem/mag.c ../mem/slab.c -pthread
