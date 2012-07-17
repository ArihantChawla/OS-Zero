#include <stddef.h>
#include <stdint.h>
#include <zero/cdecl.h>
#include <zero/param.h>
#if defined(__i386__)
#include <kern/ia32/link.h>
#include <kern/ia32/vm.h>
#endif
#include <kern/mem/mag.h>

#if (PTRBITS == 32)
#include <kern/mem/slab32.h>

#define maghdrnum(ptr)                                                  \
    ((uintptr_t)ptr >> SLABMINLOG2)
#define magadr(ptr)                                                     \
    ((void *)((uintptr_t)(ptr) & ~(SLABMIN - 1)))
struct maghdr  _maghdrtab[1U << (PTRBITS - SLABMINLOG2)] ALIGNED(PAGESIZE);
struct maghdr *_freehdrtab[PTRBITS];
long           _freelktab[PTRBITS];

void *
memalloc(unsigned long nb, long flg)
{
    unsigned long  bkt;
    struct maghdr *hdr;
    void          *ret;
    uint8_t       *u8ptr;
    long           l;
    long           n;
    long           incr;

    nb = max(PAGESIZE, nb);
    if (nb > SLABMIN) {
        ret = vmmapvirt((uint32_t *)&_pagetab,
                        slaballoc(virtslabtab, virthdrtab, nb, 0), nb, flg);
        hdr = &_maghdrtab[maghdrnum(ret)];
        hdr->n = hdr->ndx = 0;
    } else {
        bkt = slabbkt(nb);
        maglk(bkt);
        hdr = _freehdrtab[bkt];
        if (hdr) {
            ret = magpop(hdr);
            if (ret && magfull(hdr)) {
                _freehdrtab[bkt] = NULL;
            }
        } else {
            ret = u8ptr = slaballoc(virtslabtab, virthdrtab, nb, 0);
            hdr = &_maghdrtab[maghdrnum(ret)];
            n = 1UL << (SLABMINLOG2 - bkt);
            incr = 1L << bkt;
            hdr->n = n;
            hdr->ndx = 0;
            for (l = 0 ; l < n ; l++) {
                hdr->ptab[l] = u8ptr;
                u8ptr += incr;
            }
            ret = magpop(hdr);
            _freehdrtab[bkt] = hdr;
        }
        magunlk(bkt);
    }

    return ret;
}

void
kfree(void *ptr)
{
    struct maghdr  *hdr = &_maghdrtab[maghdrnum(ptr)];
    struct slabhdr *shdr;
    unsigned long   bkt;

    if (!hdr->n) {
        slabfree(virtslabtab, virthdrtab, ptr);
    } else {
        magpush(hdr, ptr);
        if (magempty(hdr)) {
            shdr = &virthdrtab[maghdrnum(ptr)];
            slabfree(virtslabtab, virthdrtab, magadr(ptr));
            bkt = slabgetbkt(shdr);
            maglk(bkt);
            if (hdr == _freehdrtab[bkt]) {
                _freehdrtab[bkt] = NULL;
            }
            magunlk(bkt);
        }
    }

    return;
}

#endif /* PTRBITS == 32 */

