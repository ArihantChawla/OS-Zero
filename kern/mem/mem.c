#include <stdint.h>
#include <zero/param.h>
#if defined(__x86_64__) || defined(__amd64__)
#include <kern/mem/slab64.h>
#elif defined(__i386__)
#include <kern/mem/slab32.h>
#include <kern/unit/ia32/link.h>
#include <kern/unit/ia32/vm.h>
#elif defined(__arm__)
#include <kern/unit/arm/link.h>
#endif

extern void pageinit(uintptr_t, unsigned long);

/* TODO: this will _not_ work on 64-bit */
struct slabhdr  virthdrtab[1U << (PTRBITS - SLABMINLOG2)] ALIGNED(PAGESIZE);
struct slabhdr *virtslabtab[PTRBITS] ALIGNED(PAGESIZE);

void
meminit(uintptr_t base, unsigned long nbphys)
{
    pageinit(base, nbphys);
    slabinit(virtslabtab, virthdrtab,
             (unsigned long)&_epagetab, (char *)KERNVIRTBASE - &_epagetab);
//    swapinit(0, 0x00000000, 1024);

    return;
}

