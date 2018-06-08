#include <stddef.h>
#include <zero/trix.h>
#include <kern/mem/zone.h>
#include <kern/mem/mem.h>
#include <kern/unit/ia32/vm.h>

//extern struct memzone memzonetab[MEM_PHYS_ZONE];
//extern struct memzone memvirtzone;
extern struct memzone memzonetab[MEM_ZONES];

struct page;
unsigned long pageinitphyszone(uintptr_t base, struct page **zone,
                               unsigned long nb);
unsigned long pageaddphyszone(uintptr_t base, struct page **zone,
                              unsigned long nb);

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
    pageinitphys((uintptr_t)&_epagetab,
                 lim - (size_t)&_epagetab);
    lim = max(nbvirt, KERNVIRTBASE);
    //    meminitzone((uintptr_t)&_epagetab, lim - (size_t)&_epagetab);
    vminitvirt(&_pagetab, &_epagetab, nbvirt, PAGEWRITE);
#elif defined(__x86_64__) || defined(__amd64__)
#error implement x86-64 memory management
#endif
//    meminitbuf();
//    swapinit(0, 0x00000000, 1024);

    return;
}

