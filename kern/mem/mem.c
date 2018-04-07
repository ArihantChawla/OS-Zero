#include <stddef.h>
#include <zero/trix.h>
#include <kern/unit/ia32/vm.h>

extern struct memzone   memphyszone;
extern struct memzone   memvirtzone;

void
meminit(size_t nbphys, size_t nbvirt)
{
    size_t    lim = max(nbphys, KERNVIRTBASE);
    uintptr_t adr;

    vmmapseg(&_pagetab,
             (uint32_t)nbvirt,
             (uint32_t)nbvirt,
             (uint32_t)lim,
             PAGEPRES | PAGEWRITE);
#if (defined(__i386__) && !defined(__x86_64__) && !defined(__amd64__))  \
    || defined(__arm__)
    meminitphys(&memphyszone, (uintptr_t)&_epagetab,
               lim - (size_t)&_epagetab);
    lim = max(nbvirt, KERNVIRTBASE);
    //    meminitzone((uintptr_t)&_epagetab, lim - (size_t)&_epagetab);
    meminitzone(&memvirtzone, lim);
#elif defined(__x86_64__) || defined(__amd64__)
#error implement x86-64 memory management
#endif
//    meminitbuf();
//    swapinit(0, 0x00000000, 1024);

    return;
}

