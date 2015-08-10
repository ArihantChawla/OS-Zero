#! /bin/sh

# clang doesn't support everything it would need to for the kernel to build
#export CC="clang -no-integrated-as"
#export AS="clang -integrated-as"
export CC=gcc
export AS=gcc
export LD=ld

./clean.sh
# -fno-builtin
$CC -Wundef -DNEWSIGNAL=1 -DNEWMP=1 -DNEWTMR=1 -DFASTINTR=1 -DNEWCLI=1 -DKLUDGE=1 -DMEMTEST=0 -D_REENTRANT=1 -DQEMU=1 -g -DKERN32BIT=1 -D__KERNEL__=1 -D__MTKERNEL__ -march=pentium-mmx -m32 -fno-asynchronous-unwind-tables -ffreestanding -static -Wall -I../../../usr/lib -I../../../usr/lib -I../../.. -I../../../usr/lib/c -g -m32 -O0 -fno-omit-frame-pointer -fno-stack-protector -c *.c ../../*.c ../x86/*.c ../../proc/*.c ../../mem/*.c ../../io/*.c ../../io/drv/chr/cons.c ../../io/drv/pc/*.c ../../io/net/*.c ../i387/math.c ../../../usr/contrib/Zeno/plasma24grid.c
#../../../usr/lib/zero/randlfg2.c ../../../usr/lib/zero/randmt32.c ../../../usr/lib/zero/x86/cpuid.c
$AS -DNEWSIGNAL=1 -DNEWMP=1 -DNEWTMR=1 -DFASTINTR=1 -DNEWCLI=1 -DQEMU=1 -g -D__KERNEL__=1 -D__MTKERNEL__ -march=pentium-mmx -ffreestanding -static -Wall -I../../.. -I../../../usr/lib -I../../../usr/lib/c -g -m32 -O0 -fno-omit-frame-pointer -fno-stack-protector -msse -c *.S
$LD -nostdlib -nodefaultlibs -g -m32 -static -T kern.lds -m elf_i386 -Map zero.sym -o zero *.o
#objdump --line-numbers --source zero.elf > zero.lst
#nm -n zero.elf > zero.sym
#objcopy -O elf32-i386 zero.elf zero
#./mkcdimg.sh

