gcc -O0 -fno-builtin -g -Wall -DMTSAFE=0 -DMEMTEST=1 -I../.. -I../../usr/lib -o memtest memtest.c ../mem/mag.c ../mem/slab.c -pthread
