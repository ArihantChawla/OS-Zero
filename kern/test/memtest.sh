gcc -m32 -O0 -fno-builtin -g -Wall -DMEMHACKS=1 -DMEMFREECHK=0 -DMEMTEST=1 -I../.. -I../../usr/lib -o memtest memtest.c ../mem/mem.c ../mem/mag32.c ../mem/slab32.c -pthread
