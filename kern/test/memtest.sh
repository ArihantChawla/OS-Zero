gcc -g -Wall -DMEMFREECHK=0 -DMEMTEST=1 -m32 -I../.. -I../../usr/lib -O0 -o memtest memtest.c ../mem/mem.c ../mem/mag32.c ../mem/slab32.c -pthread
