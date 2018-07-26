#include <stddef.h>
#include <stdint.h>
#include <zero/trix.h>
#include <kern/mem/page.h>
#include <kern/mem/mem.h>
#include <kern/mem/zone.h>
#include <kern/unit/ia32/vm.h>

extern struct memzone k_memzonetab[MEM_ZONES];

void
meminit(size_t nbphys, size_t nbvirt)
{
    size_t    lim = max(nbphys, KERNVIRTBASE);
    uintptr_t adr;

    vmmapseg((uint32_t)nbvirt,
             (uint32_t)nbvirt,
             (uint32_t)lim,
             PAGEPRES | PAGEWRITE);
#if defined(__i386__) && !defined(__x86_64__) && !defined(__amd64__)
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

