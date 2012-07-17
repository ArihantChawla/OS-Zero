#include <stdint.h>
#include <zero/cdecl.h>
#include <zero/param.h>
#include <kern/mem/page.h>
#if defined(__i386__)
#include <kern/mem/slab32.h>
#include <kern/ia32/link.h>
#elif defined(__arm__)
#endif

#if (PTRBITS == 32)

#define KERNVIRTBASE 0xc0000000U

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

#endif /* PTRBITS == 32 */

