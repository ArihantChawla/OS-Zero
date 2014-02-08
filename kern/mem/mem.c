#include <stdint.h>
#include <zero/param.h>
#include <kern/mem/mem.h>
#include <kern/mem/slab.h>
#if defined(__i386__) && !defined(__x86_64__) && !defined(__amd64__)
#include <kern/mem/slab32.h>
#include <kern/unit/ia32/link.h>
#include <kern/unit/ia32/vm.h>
#elif defined(__arm__)
#include <kern/unit/arm/link.h>
#endif

extern struct memzone slabvirtzone;

void
meminit(uintptr_t base, unsigned long nbphys)
{
#if (defined(__i386__) && !defined(__x86_64__) && !defined(__amd64__))  \
    || defined(__arm__)
//    slabinit((unsigned long)&_ebss, (char *)KERNVIRTBASE - &_ebss);
    slabinit(&slabvirtzone, (unsigned long)&_ebss,
             max(nbphys, DEVMEMBASE) - (uintptr_t)&_ebss);
#elif defined(__x86_64__) || defined(__amd64__)
#error implement x86-64 memory management
#endif

//    swapinit(0, 0x00000000, 1024);

    return;
}

