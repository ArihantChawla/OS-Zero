gcc -O0 -m32 -fno-builtin -g -Wall -DMEMFREECHK=0 -DMEMTEST=1 -I../.. -I../../usr/lib -o memtest memtest.c ../mem/mem.c ../mem/mag32.c ../mem/slab32.c -pthread
