#! /bin/sh

export CC=gcc
#export CC=clang
export AS=gcc

./clean.sh
#$CC -DQEMU=1 -g -D__KERNEL__=1 -D__MTKERNEL__ -ffreestanding -static -Wall -I../.. -I../../usr/lib -I../../usr/lib/c -g -m32 -O -fno-omit-frame-pointer -fno-stack-protector -fno-builtin -c *.c *.S ../*.c ../mem/*.c ../io/drv/pc/*.c ../io/drv/pc/ps2/*.c
$CC -DQEMU=1 -g -D__KERNEL__=1 -D__MTKERNEL__ -ffreestanding -static -Wall -I../.. -I../../usr/lib -I../../usr/lib/c -g -m32 -O -fno-omit-frame-pointer -fno-stack-protector -fno-builtin -c *.c ../*.c ../mem/*.c ../io/drv/pc/*.c ../io/drv/pc/ps2/*.c
$AS -DQEMU=1 -g -D__KERNEL__=1 -D__MTKERNEL__ -ffreestanding -static -Wall -I../.. -I../../usr/lib -I../../usr/lib/c -g -m32 -O -fno-omit-frame-pointer -fno-stack-protector -fno-builtin -c *.S
#$CC -static -D__KERNEL__=1 -DMULTITASK=0 -DDEVEL=1 -Wall -I. -I./usr/include -I./usr/lib/c -g -m32 -O0 -fno-omit-frame-pointer -fno-stack-protector -fno-builtin -nostdlib -c mach/ia32/mp/mpentry.S
ld -g -static -T kern.lds -m elf_i386 -Map zero.sym -o zero *.o
#objdump --line-numbers --source zero.elf > zero.lst
#nm -n zero.elf > zero.sym
#objcopy -O elf32-i386 zero.elf zero

