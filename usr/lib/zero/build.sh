#! /bin/sh

CC=gcc
CCBUILDFLAGS="-DZERONEWFMTX=1 -D_REENTRANT=1 -D_ZERO_SOURCE=1 -D_GNU_SOURCE=1 -DZEROCOND=1 -DSHUNTC=1 -DNVALGRIND -DZEROTHR=1 -DMEMMULTITAB=0 -DMEMNEWHASH=1 -DMEMLFHASH=1 -msse2"
#CC=clang
#CCWARNFLAGS="-D_FORTIFY_SOURCE=2 -Wall -Wundef -Wextra -Wpointer-arith -Wuninitialized -Winit-self -Wmissing-include-dirs -Wparentheses -Wswitch-default -Wswitch-bool -Wunused-parameter -Wstrict-aliasing -Wstrict-overflow -Wsuggest-attribute=pure -Wsuggest-attribute=const -Wsuggest-attribute=noreturn -Wsuggest-attribute=format -Wtrampolines -Wfloat-equal -Wshadow -Wstack-usage=16384 -Wunsafe-loop-optimizations -Wbad-function-cast -Wc99-c11-compat -Wcast-qual -Wcast-align -Wwrite-strings -Wconversion -Wdate-time -Wjump-misses-init -Wlogical-op -Waggregate-return -Wstrict-prototypes -Wnormalized -Wopenmp-simd -Wpacked -Wpadded -Wredundant-decls -Wnested-externs -Winline -Wvector-operation-performance -Wvla -Wstack-protector -Woverlength-strings -Wunsuffixed-float-constants -Wno-designated-init -Wdeclaration-after-statement -Wformat-security -Wmissing-format-attribute"
CCWARNFLAGS="-D_FORTIFY_SOURCE=2 -Wall -Wundef -Wpointer-arith -Wuninitialized -Winit-self -Wmissing-include-dirs -Wparentheses -Wswitch-default -Wswitch-bool -Wunused-parameter -Wstrict-aliasing -Wstrict-overflow -Wsuggest-attribute=pure -Wsuggest-attribute=const -Wsuggest-attribute=noreturn -Wsuggest-attribute=format -Wtrampolines -Wfloat-equal -Wshadow -Wstack-usage=16384 -Wunsafe-loop-optimizations -Wbad-function-cast -Wc99-c11-compat -Wcast-align -Wwrite-strings -Wdate-time -Wjump-misses-init -Wlogical-op -Waggregate-return -Wstrict-prototypes -Wnormalized -Wopenmp-simd -Wpacked -Wpadded -Wredundant-decls -Wnested-externs -Winline -Wvector-operation-performance -Wvla -Wstack-protector -Woverlength-strings -Wno-designated-init -Wdeclaration-after-statement -Wformat-security -Wmissing-format-attribute"

#$CC -Wextra -D_REENTRANT=1 -DSHUNTC=1 -DNVALGRIND -DZERONEWMTX=0 -DZEROMALLOC=0 -DPTHREAD=1 -D_REENTRANT -g -Wall -O -I.. -I.. -fPIC -shared -o libzero.so daemon.c data.c file.c futex.c hash.c maplk.c randk.c randlfg2.c randlfg.c randmt32.c randmt64.c unix.c util.c shuntc.c shunt.c -pthread
#$CC $CCWARNFLAGS -m32 -D_REENTRANT=1 -D_ZERO_SOURCE=1 -D_GNU_SOURCE=1 -DZEROCOND=1 -DZEROMTX=1 -DSHUNTC=1 -DNVALGRIND -DZEROMTX=1 -DZEROMALLOC=0 -DPTHREAD=1 -DZEROTHR=1 -g -O -I.. -fPIC -shared -o libzero32.so *.c ia32/*.c -pthread

$CC $CCBUILDFLAGS $CCWARNFLAGS -g -O -I.. -fPIC -shared -o libzero.so *.c x86-64/*.c x86-64/*.S -pthread

