#! /bin/sh

# clang doesn't support everything it would need to for the kernel to build
#export CC="clang -no-integrated-as"
#export AS="clang -integrated-as"
export CC=gcc
export AS=gcc
export LD=ld

./clean.sh
$CC -DASMSWITCH=1 -DKLUDGE=1 -DMEMTEST=0 -D_REENTRANT=1 -DQEMU=1 -g -DKERN32BIT=1 -D__KERNEL__=1 -D__MTKERNEL__ -march=pentium-mmx -m32 -fno-asynchronous-unwind-tables -ffreestanding -static -Wall -I../../.. -I../../../usr/lib -I../../../usr/lib/c -g -m32 -O -fno-omit-frame-pointer -fno-stack-protector -fno-builtin -c *.c ../../*.c ../x86/*.c ../../proc/*.c ../../mem/*.c ../../io/*.c ../../io/drv/chr/cons.c ../../io/drv/pc/*.c ../i387/math.c ../../../usr/contrib/Zeno/plasma24grid.c
$AS -DASMSWITCH=1 -DQEMU=1 -g -D__KERNEL__=1 -D__MTKERNEL__ -ffreestanding -static -Wall -I../../.. -I../../../usr/lib -I../../../usr/lib/c -g -m32 -O -fno-omit-frame-pointer -fno-stack-protector -fno-builtin -c *.S
$LD -nostdlib -nodefaultlibs -g -static -T kern.lds -m elf_i386 -Map zero.sym -o zero *.o
#objdump --line-numbers --source zero.elf > zero.lst
#nm -n zero.elf > zero.sym
#objcopy -O elf32-i386 zero.elf zero
#./mkcdimg.sh

