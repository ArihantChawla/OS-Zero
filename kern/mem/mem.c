#include <kern/conf.h>
#include <stdint.h>
#include <zero/param.h>
#include <zero/trix.h>
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
meminit(unsigned long nbphys)
{
    unsigned long lim = min(nbphys, KERNVIRTBASE);

#if (defined(__i386__) && !defined(__x86_64__) && !defined(__amd64__))  \
    || defined(__arm__)
    slabinit(&slabvirtzone, (unsigned long)&_epagetab,
             lim - (unsigned long)&_epagetab);
#elif defined(__x86_64__) || defined(__amd64__)
#error implement x86-64 memory management
#endif

//    swapinit(0, 0x00000000, 1024);

    return;
}

