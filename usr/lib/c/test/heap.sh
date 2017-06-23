#! /bin/sh

#CC=clang
#CCFLAGS="-Wall -Wundef -Wextra -Wpointer-arith"
CC=gcc
#CCOPTFLAGS="-DMEMBIGHASHTAB=0 -DMEMHASHLOCK=0 -DMEMNEWHASH=1 -DMEMBITFIELD=0 -DMEMSTAT=0 -DMEMDEBUG=1 -DMEMDEBUGLOCK=0 -DMEMTEST=1 -DMEMHASHLOOP=0 -DMEMBIGARRAYHASH=0 -DMEMARRAYHASH=0 -DMEMHASH=0 -DMEMHUGELOCK=1 -DMEMMULTITAB=0 -DMEMVALGRIND=1 -O0 -fno-builtin" # -fno-tree-loop-distribute-patterns -fno-omit-frame-pointer"
CCOPTFLAGS="-DMEMBKTLOCK=0 -DMEMSPINBITLOCK=0 -O -fno-builtin -DMEMBLKHDR=0 -DMEMSIGNAL=0 -DMEMBIGHASH=0 -DMEMSTAT=1 -DMEMBUFDEBUG=0 -DMEMHASHSUBTABS=0 -DMEMBIGPAGES=0 -DMEMUNMAP=1 -DMEMEMPTYTLS=1 -DMEMDEADBINS=0 -DMEMCACHECOLOR=0 -DMEMBUFSTACK=1 -DMEMBUFRELMAP=0 -DMEMDYNTLS=1 -DMEMNEWHASHTAB=0 -DMEMNOSBRK=1 -DGNUMALLOC=0 -DMEMHASHMURMUR=0 -DMEMHASHNREF=0 -DMEMHASHNACT=0 -DMEMDEBUGDEADLOCK=0 -DMEMMULTITAB=0 -DMEMTINYHASHTAB=0 -DMEMSMALLHASHTAB=0 -DMEMHUGEHASHTAB=0 -DMEMBIGHASHTAB=0 -DMEMNEWHASH=1 -DMEMBITFIELD=0 -DMEMDEBUG=1 -DMEMDEBUGLOCK=0 -DMEMTEST=0 -DMEMTESTSTACK=0 -DMEMVALGRIND=1 -finline-functions -fno-builtin-malloc -fno-builtin-calloc -fno-builtin-free -fno-builtin-realloc -fno-builtin-memalign"
#CCOPTFLAGS="-DMEMSPINBITLOCK=1 -O -fno-builtin -DMEMBLKHDR=1 -DMEMSIGNAL=0 -DMEMBIGHASH=0 -DMEMSTAT=1 -DMEMBUFDEBUG=0 -DMEMHASHSUBTABS=1 -DMEMBIGPAGES=0 -DMEMUNMAP=1 -DMEMEMPTYTLS=1 -DMEMDEADBINS=0 -DMEMCACHECOLOR=1 -DMEMBUFSTACK=1 -DMEMBUFRELMAP=1 -DMEMDYNTLS=1 -DMEMNEWHASHTAB=0 -DMEMNOSBRK=1 -DGNUMALLOC=0 -DMEMHASHMURMUR=0 -DMEMHASHNREF=0 -DMEMHASHNACT=0 -DMEMDEBUGDEADLOCK=0 -DMEMMULTITAB=0 -DMEMTINYHASHTAB=0 -DMEMSMALLHASHTAB=0 -DMEMHUGEHASHTAB=0 -DMEMBIGHASHTAB=0 -DMEMNEWHASH=1 -DMEMBITFIELD=0 -DMEMDEBUG=0 -DMEMDEBUGLOCK=0 -DMEMTEST=0 -DMEMTESTSTACK=0 -DMEMVALGRIND=1 -finline-functions -fno-builtin-malloc -fno-builtin-calloc -fno-builtin-free -fno-builtin-realloc -fno-builtin-memalign"
#CCOPTFLAGS="-DMEMSIGNAL=0 -DMEMBIGHASH=0 -DMEMSTAT=1 -DMEMBUFDEBUG=0 -DMEMHASHSUBTABS=0 -DMEMBIGPAGES=0 -DMEMUNMAP=1 -DMEMEMPTYTLS=1 -DMEMDEADBINS=0 -DMEMCACHECOLOR=0 -DMEMBUFSTACK=1 -DMEMBFHASH=1 -DMEMBUFRELMAP=1 -DMEMDYNTLS=1 -DMEMNEWHASHTAB=0 -DMEMNOSBRK=1 -DGNUMALLOC=0 -DMEMHASHMURMUR=0 -DMEMHASHNREF=0 -DMEMHASHNACT=0 -DMEMDEBUGDEADLOCK=0 -DMEMMULTITAB=0 -DMEMTINYHASHTAB=0 -DMEMSMALLHASHTAB=0 -DMEMHUGEHASHTAB=0 -DMEMBIGHASHTAB=0 -DMEMNEWHASH=1 -DMEMBITFIELD=0 -DMEMDEBUG=0 -DMEMDEBUGLOCK=0 -DMEMTEST=0 -DMEMTESTSTACK=0 -DMEMVALGRIND=1 -O -finline-functions -fno-builtin-memset -fno-builtin-malloc -fno-builtin-calloc -fno-builtin-free -fno-builtin-realloc -fno-builtin-memalign" # -fno-tree-loop-distribute-patterns -fno-omit-frame-pointer"
#LDFLAGS="-lpthread"
LDFLAGS=""
CCFLAGS="-D_GNU_SOURCE -D_FORTIFY_SOURCE=2 -Wall -Wundef -Wextra -Wpointer-arith -Wuninitialized -Winit-self -Wmissing-include-dirs -Wparentheses -Wswitch-default -Wswitch-bool -Wunused-parameter -Wstrict-aliasing -Wsuggest-attribute=pure -Wsuggest-attribute=const -Wsuggest-attribute=noreturn -Wsuggest-attribute=format -Wtrampolines -Wfloat-equal -Wshadow -Wstack-usage=16384 -Wunsafe-loop-optimizations -Wbad-function-cast -Wc99-c11-compat -Wcast-align -Wwrite-strings -Wdate-time -Wjump-misses-init -Wlogical-op -Waggregate-return -Wstrict-prototypes -Wnormalized -Wopenmp-simd -Wpacked -Wredundant-decls -Wnested-externs -Winline -Wvector-operation-performance -Wvla -Wstack-protector -Woverlength-strings -Wno-designated-init -Wdeclaration-after-statement -Wformat-security -Wmissing-format-attribute"

# As of October 24 2011, gcc -O2 produces slower code than -O // vendu
# - same thing with -finline-functions and -funroll-loops
# UPDATE February 22 2017, -finline-functions is worth it :)
$CC $CCFLAGS $CCOPTFLAGS -DZEROFUTEX=0 -DZEROMALLOC=1 -DPTHREAD=1 -D_REENTRANT -g -Wall -I../.. -o heap heap.c $LDFLAGS -pthread
$CC $CCFLAGS $CCOPTFLAGS -DZEROFUTEX=0 -DZEROMALLOC=1 -DPTHREAD=1 -D_REENTRANT -g -Wall -I../.. -o llheap lockless_allocator/ll_alloc.c heap.c $LDFLAGS -pthread
$CC $CCFLAGS $CCOPTFLAGS -DZEROFUTEX=0 -DZEROMALLOC=1 -DPTHREAD=1 -D_REENTRANT -g -Wall -I../.. -o tcheap heap.c -Wl,-L/usr/local/lib -ltcmalloc $LDFLAGS -pthread
$CC $CCFLAGS $CCOPTFLAGS -DZEROFUTEX=0 -DZEROMALLOC=1 -DPTHREAD=1 -D_REENTRANT -g -Wall -I../.. -o jeheap heap.c -Wl,-L/usr/local/lib -ljemalloc $LDFLAGS -pthread
$CC $CCFLAGS $CCOPTFLAGS -DX11VIS=0 -DZEROFUTEX=0 -DZEROMALLOC=1 -DPTHREAD=1 -D_REENTRANT -g -Wall -fno-builtin -fno-tree-loop-distribute-patterns -DZEROTHR=0 -fno-omit-frame-pointer -I../.. -I../../../.. -I../../../lib -o zheap heap.c ../../zero/priolk.c ../../zero/hash.c ../../zero/mem.c ../../zero/_mem.c ../malloc.c $LDFLAGS -pthread
#$CC $CCFLAGS $CCOPTFLAGS -m32 -DX11VIS=0 -DZEROFUTEX=0 -DZEROMALLOC=1 -DPTHREAD=1 -D_REENTRANT -g -Wall -fno-builtin -fno-tree-loop-distribute-patterns -DZEROTHR=0 -fno-omit-frame-pointer -I../.. -I../../../.. -I../../../lib -o zheap32 heap.c ../../zero/priolk.c ../../zero/hash.c ../../zero/mem.c ../../zero/_mem.c ../malloc.c $LDFLAGS -pthread
##$CC $CCFLAGS $CCOPTFLAGS -DX11VIS=0 -DMALLOCTRACE=1 -DZEROFUTEX=0 -DZEROMALLOC=1 -DPTHREAD=1 -D_REENTRANT -g -Wall -I../../../.. -I../../../lib -o tzheap heap.c ../execinfo.c ../../zero/priolk.c ../../zero/hash.c ../../zero/mem.c ../../zero/_mem.c ../malloc.c $LDFLAGS -pthread
$CC $CCFLAGS $CCOPTFLAGS -DZEROFUTEX=0 -DZEROMALLOC=1 -DPTHREAD=1 -D_REENTRANT -DX11VIS=1 -g -Wall -I../.. -I../../../.. -I../../../lib -o xzheap heap.c ../../zero/priolk.c ../../zero/hash.c ../../zero/mem.c ../../zero/_mem.c ../malloc.c -Wl,-lX11 $LDFLAGS -pthread
$CC $CCFLAGS $CCOPTFLAGS -DZEROFUTEX=0 -DZEROMALLOC=1 -DPTHREAD=1 -D_REENTRANT -DX11VIS=1 -g -Wall -I../.. -I../../../.. -I../../../lib -o t-test1 ../../zero/priolk.c ../../zero/hash.c ../../zero/mem.c ../../zero/_mem.c ../malloc.c t-test1.c -Wl,-lX11 $LDFLAGS -pthread
$CC $CCFLAGS $CCOPTFLAGS -DZEROFUTEX=0 -DZEROMALLOC=1 -DPTHREAD=1 -D_REENTRANT -DX11VIS=1 -g -Wall -I../.. -I../../../.. -I../../../lib -o tmem ../../zero/priolk.c ../../zero/hash.c ../../zero/mem.c ../../zero/_mem.c ../malloc.c tmem.c ../../zero/_mem.c -Wl,-lX11 $LDFLAGS -pthread
#$CC $CCFLAGS $CCOPTFLAGS -DPTHREAD=1 -DX11VIS=1 -D_REENTRANT -g -Wall -I../../../.. -I../../../lib /../include -o xheap heap.c ../stdlib/../../zero/priolk.c ../../zero/hash.c ../../zero/mem.c ../../zero/_mem.c ../malloc.c $LDFLAGS -pthread -lX11
#$CC $CCFLAGS $CCOPTFLAGS -DPTHREAD=1 -D_REENTRANT -g -Wall -I../../../.. -I../../../lib /../include -o jeheap heap.c libjemalloc.a $LDFLAGS -pthread
$CC $CCFLAGS $CCOPTFLAGS -DX11VIS=0 -DZEROFUTEX=0 -DZEROMALLOC=1 -DPTHREAD=1 -D_REENTRANT -DDEVEL=0 -g -Wall -fno-builtin -fno-builtin -fno-tree-loop-distribute-patterns -shared -fPIC -I../../../.. -I../../../lib -o zmalloc.so -I../.. ../../zero/priolk.c ../../zero/hash.c ../../zero/mem.c ../../zero/_mem.c ../malloc.c $LDFLAGS -pthread







