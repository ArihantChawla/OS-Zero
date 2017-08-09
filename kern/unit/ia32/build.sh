#! /bin/sh

# clang doesn't support everything it would need to for the kernel to build
#CC="clang -no-integrated-as"
#AS="clang -integrated-as"
CC=gcc
AS=gcc
LD=ld

CFLAGS="-m32 -Xlinker -mno-red-zone -DZERONEWFMTX=1"
CCWARNFLAGS="-Wall -Wundef -Wextra -Wpointer-arith -Wuninitialized"
#CCWARNFLAGS="-D_FORTIFY_SOURCE=2 -Wall -Wundef -Wextra -Wpointer-arith -Wuninitialized -Winit-self -Wmissing-include-dirs -Wparentheses -Wswitch-default -Wswitch-bool -Wunused-parameter -Wstrict-aliasing -Wsuggest-attribute=pure -Wsuggest-attribute=const -Wsuggest-attribute=noreturn -Wsuggest-attribute=format -Wtrampolines -Wfloat-equal -Wshadow -Wstack-usage=16384 -Wunsafe-loop-optimizations -Wbad-function-cast -Wc99-c11-compat -Wcast-align -Wwrite-strings -Wdate-time -Wjump-misses-init -Wlogical-op -Waggregate-return -Wstrict-prototypes -Wnormalized -Wopenmp-simd -Wpacked -Wredundant-decls -Wnested-externs -Winline -Wvector-operation-performance -Wvla -Wstack-protector -Woverlength-strings -Wno-designated-init -Wdeclaration-after-statement -Wformat-security -Wmissing-format-attribute"

./clean.sh
$CC $CFLAGS $CCWARNFLAGS -Wno-unused -D__ZEROLIBC__=1 -D_ZERO_SOURCE=1 -DNEWSIGNAL=1 -DNEWMP=1 -DNEWTMR=1 -DFASTINTR=1 -DNEWCLI=1 -DKLUDGE=1 -DMEMTEST=0 -D_REENTRANT=1 -DQEMU=1 -g -DKERN32BIT=1 -D__KERNEL__=1 -D__MTKERNEL__ -march=pentium-mmx -fno-asynchronous-unwind-tables -ffreestanding -static -I../../../usr/lib -I../../../usr/lib -I../../.. -I../../../usr/lib/c -O -fno-omit-frame-pointer -c ../../../usr/lib/zero/hash.c *.c ../../*.c ../x86/*.c ../../proc/*.c ../../mem/*.c ../../io/*.c ../../io/drv/chr/cons.c ../../io/drv/pc/*.c ../../math/i387/math.c ../../../usr/contrib/Zeno/plasma24grid.c ../../../usr/lib/zero/fastudiv.c 
#../../../usr/lib/zero/randlfg2.c ../../../usr/lib/zero/randmt32.c ../../../usr/lib/zero/x86/cpuid.c
$AS $CFLAGS -DZEROMTX=1 -D_ZERO_SOURCE=1 -DNEWSIGNAL=1 -DNEWMP=1 -DNEWTMR=1 -DNEWCLI=1 -DQEMU=1 -g -D__ZEROKERNEL___=1 -D__KERNEL__=1 -D__MTKERNEL__ -march=pentium-mmx -ffreestanding -static -I../../.. -I../../../usr/lib -I../../../usr/lib/c/include -I../../../usr/lib/c -I../../../usr/lib/c/zlibc -O -fno-omit-frame-pointer -msse -c ../x86/boot.S *.S
$LD --check-sections -nostdlib -nodefaultlibs -g -static -T kern.lds -m elf_i386 -Map zero.sym -o zero *.o
#objdump --line-numbers --source zero.elf > zero.lst
#nm -n zero.elf > zero.sym
#objcopy -O elf32-i386 zero.elf zero

