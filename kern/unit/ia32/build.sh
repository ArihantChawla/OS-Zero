#! /bin/sh

export CC=gcc
export AS=gcc
export LD=ld

./clean.sh
#$CC -DQEMU=1 -g -D__KERNEL__=1 -D__MTKERNEL__ -ffreestanding -static -Wall -I../.. -I../../usr/lib -I../../usr/lib/c -g -m32 -O -fno-omit-frame-pointer -fno-stack-protector -fno-builtin -c *.c *.S ../*.c ../mem/*.c ../io/drv/pc/*.c ../io/drv/pc/ps2/*.c
$CC -DMEMTEST=0 -D_REENTRANT=1 -DQEMU=1 -g -DKERN32BIT=1 -D__KERNEL__=1 -D__MTKERNEL__ -march=pentium-mmx -m32 -fno-asynchronous-unwind-tables -ffreestanding -static -Wall -I../../.. -I../../../usr/lib -I../../../usr/lib/c -g -m32 -O -fno-omit-frame-pointer -fno-stack-protector -fno-builtin -c *.c ../../*.c ../x86/*.c ../../proc/*.c ../../mem/*.c ../../io/*.c ../../io/drv/chr/con.c ../../io/drv/pc/*.c ../../io/drv/pc/ps2/*.c ../i387/math.c ../../../usr/contrib/Zeno/plasma24grid.c
$AS -DQEMU=1 -g -D__KERNEL__=1 -D__MTKERNEL__ -ffreestanding -static -Wall -I../../.. -I../../../usr/lib -I../../../usr/lib/c -g -m32 -O -fno-omit-frame-pointer -fno-stack-protector -fno-builtin -c *.S
#$CC -static -D__KERNEL__=1 -DMULTITASK=0 -DDEVEL=1 -Wall -I. -I./usr/include -I./usr/lib/c -g -m32 -O -fno-omit-frame-pointer -fno-stack-protector -fno-builtin -nostdlib -c mach/ia32/mp/mpentry.S
#$LD -nostdlib -nodefaultlibs -g -static -T kern.lds -m elf_i386 -Map zero.sym -o zero *.o
$LD -nostdlib -nodefaultlibs -g -static -T kern.lds -m elf_i386 -Map zero.sym -o zero *.o
#objdump --line-numbers --source zero.elf > zero.lst
#nm -n zero.elf > zero.sym
#objcopy -O elf32-i386 zero.elf zero
#./mkcdimg.sh

