#include <kern/conf.h>
#include <stddef.h>
#include <stdint.h>
#include <zero/param.h>
#include <zero/trix.h>
#include <kern/util.h>
#include <kern/mem/vm.h>
#include <kern/mem/pool.h>
#include <kern/mem/slab.h>
#if defined(__i386__) && !defined(__x86_64__) && !defined(__amd64__)
#include <kern/mem/slab32.h>
#include <kern/unit/x86/link.h>
#elif defined(__arm__)
#include <kern/unit/arm/link.h>
#endif

extern struct mempool memphyspool;

void
meminit(size_t nbphys)
{
    size_t lim = max(nbphys, KERNVIRTBASE);

#if (defined(__i386__) && !defined(__x86_64__) && !defined(__amd64__))  \
    || defined(__arm__)
    vminitvirt(&_pagetab, &_epagetab,
              lim - (uint32_t)&_epagetab,
              PAGEWRITE);
    meminitphys(&memphyspool, (size_t)&_epagetab,
                lim - (size_t)&_epagetab);
#elif defined(__x86_64__) || defined(__amd64__)
#error implement x86-64 memory management
#endif

//    swapinit(0, 0x00000000, 1024);

    return;
}

void
meminitphys(struct mempool *physpool, uintptr_t base, size_t nbphys)
{
    struct memslab **blktab = (struct memslab **)physpool->tab;
    uintptr_t        adr = ((base & (MEMMIN - 1))
                            ? rounduppow2(base, MEMMIN)
                            : base);
    unsigned long    bkt = PTRBITS - 1;
    size_t           sz = 1UL << bkt;
    struct memslab  *slab;

    adr = meminitpool(physpool, adr, nbphys);
    nbphys -= adr - base;
    nbphys = rounddownpow2(nbphys, MEMMIN);
    vmmapseg((uint32_t *)&_pagetab, adr, adr, adr + nbphys,
             PAGEWRITE);
#if (__KERNEL__)
    kprintf("%ld kilobytes kernel virtual memory free @ 0x%lx\n",
            nbphys >> 10, adr);
#endif
    while ((nbphys) && bkt >= MEMMINLOG2) {
        if (nbphys & sz) {
            slab = memgethdr(adr, physpool);
            memslabclrinfo(slab);
            memslabclrlink(slab);
            memslabsetbkt(slab, bkt);
            memslabsetfree(slab);
            blktab[bkt] = slab;
            nbphys &= ~sz;
            adr += sz;
        }
        bkt--;
        sz >>= 1;
    }
#if (__KERNEL__ && (MEMDIAG))
    memdiag(physpool);
#endif

    return;
}

