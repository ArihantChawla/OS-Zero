#include <stdint.h>
#include <zero/param.h>
#include <kern/mem/slab.h>
#if defined(__i386__) && !defined(__x86_64__) && !defined(__amd64__)
#include <kern/mem/slab32.h>
#include <kern/unit/ia32/link.h>
#include <kern/unit/ia32/vm.h>
#elif defined(__arm__)
#include <kern/unit/arm/link.h>
#endif

void
meminit(uintptr_t base, unsigned long nbphys)
{
#if (defined(__i386__) && !defined(__x86_64__) && !defined(__amd64__))  \
    || defined(__arm__)
//    slabinit((unsigned long)&_ebss, (char *)KERNVIRTBASE - &_ebss);
    slabinit((unsigned long)&_ebss, nbphys - (uintptr_t)&_ebss);
#elif defined(__x86_64__) || defined(__amd64__)
#error implement x86-64 memory management
#endif

//    swapinit(0, 0x00000000, 1024);

    return;
}

