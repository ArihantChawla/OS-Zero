#include <stddef.h>
#include <stdint.h>
#include <zero/trix.h>
#include <kern/mem/zone.h>
#include <kern/mem/mem.h>
#include <kern/mem/vm.h>
//#include <kern/unit/ia32/vm.h>

extern struct memzone memzonetab[MEM_ZONES];

extern unsigned long pageinitphyszone(uintptr_t base, struct vmpage **zone,
                                      unsigned long nb);
extern unsigned long pageaddphyszone(uintptr_t base, struct vmpage **zone,
                                     unsigned long nb);
extern unsigned long pageinitphys(uintptr_t base, size_t nbphys);

void
meminit(size_t nbphys, size_t nbvirt)
{
    size_t    lim = max(nbphys, KERNVIRTBASE);
    uintptr_t adr;

    vmmapseg((uint32_t)nbvirt,
             (uint32_t)nbvirt,
             (uint32_t)lim,
             PAGEPRES | PAGEWRITE);
#if (defined(__i386__) && !defined(__x86_64__) && !defined(__amd64__))  \
    || defined(__arm__)
    pageinitphys((uintptr_t)&_epagetab,
                 lim - (size_t)&_epagetab);
    lim = max(nbvirt, KERNVIRTBASE);
    //    meminitzone((uintptr_t)&_epagetab, lim - (size_t)&_epagetab);
    vminitvirt(&_epagetab, nbvirt, PAGEWRITE);
#elif defined(__x86_64__) || defined(__amd64__)
#error implement x86-64 memory management
#endif
//    meminitbuf();
//    swapinit(0, 0x00000000, 1024);

    return;
}

