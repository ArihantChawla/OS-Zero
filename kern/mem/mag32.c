#include <stddef.h>
#include <stdint.h>
#include <zero/cdecl.h>
#include <zero/param.h>
#if (MEMFREECHK)
#include <zero/trix.h>
#endif
#if defined(__i386__)
#include <kern/unit/ia32/link.h>
#include <kern/unit/ia32/vm.h>
#endif
#include <kern/mem/mag.h>
#include <kern/mem/slab32.h>

#if (MEMTEST)
#include <stdio.h>
extern uint32_t pagetab[NPDE][NPTE];
#endif

struct maghdr  _maghdrtab[1UL << (PTRBITS - SLABMINLOG2)] ALIGNED(PAGESIZE);
struct maghdr *_freehdrtab[PTRBITS];
long           _freelktab[PTRBITS];
#if (MEMFREECHK)
uint8_t        _membitmap[1UL << (PTRBITS - MAGMINLOG2 - 3)];
#endif

void *
memalloc(unsigned long nb, long flg)
{
    void          *ptr = NULL;
    unsigned long  sz = 0;
    unsigned long  bkt;
    struct maghdr *mag;
    uint8_t       *u8ptr;
    unsigned long  l;
    unsigned long  n;

    nb = max(MAGMIN, nb);
    if (nb > (SLABMIN >> 1)) {
#if (MEMTEST)
        ptr = slaballoc(virtslabtab, virthdrtab, nb, flg);
#else
        ptr = vmmapvirt((uint32_t *)&_pagetab,
                        slaballoc(virtslabtab, virthdrtab, nb, flg), nb, flg);
#endif
        mag = &_maghdrtab[maghdrnum(ptr)];
        mag->n = 1;
        mag->ndx = 0;
    } else {
        bkt = slabbkt(nb);
        maglk(bkt);
        mag = _freehdrtab[bkt];
        if (mag) {
            ptr = magpop(mag);
            if (magfull(mag)) {
                if (mag->next) {
                    mag->next->prev = NULL;
                }
                _freehdrtab[bkt] = mag->next;
            }
        } else {
            sz = 1UL << bkt;
            ptr = u8ptr = slaballoc(virtslabtab, virthdrtab, sz, flg);
            n = 1UL << (SLABMINLOG2 - bkt);
            mag = &_maghdrtab[maghdrnum(ptr)];
            mag->n = n;
            mag->ndx = 0;
            mag->bkt = bkt;
#if (MEMTEST)
//            fprintf(stderr, "INIT: %ld items:", n);
#endif
            for (l = 0 ; l < n ; l++) {
                mag->ptab[l] = u8ptr;
#if (MEMTEST)
//                fprintf(stderr, " %p", u8ptr);
#endif
                u8ptr += sz;
            }
#if (MEMTEST)
//            fprintf(stderr, "\n");
#endif
            ptr = magpop(mag);
            if (_freehdrtab[bkt]) {
                _freehdrtab[bkt]->prev = mag;
            }
            mag->next = _freehdrtab[bkt];
            _freehdrtab[bkt] = mag;
        }
        magunlk(bkt);
    }
#if (MEMFREECHK)
    if (ptr) {
        if (bitset(_membitmap, (uintptr_t)ptr >> MAGMINLOG2)) {
            fprintf(stderr, "duplicate allocation %p\n", ptr);
            
            abort();
        }
        setbit(_membitmap, (uintptr_t)ptr >> MAGMINLOG2);
    }
#endif

    return ptr;
}

void
kfree(void *ptr)
{
    struct maghdr  *mag = &_maghdrtab[maghdrnum(ptr)];
    unsigned long   bkt;

    if (!ptr) {

        return;
    }
#if (MEMFREECHK)
    if (!bitset(_membitmap, (uintptr_t)ptr >> MAGMINLOG2)) {
        fprintf(stderr, "invalid free: %p\n", ptr);

        abort();
    }
    clrbit(_membitmap, (uintptr_t)ptr >> MAGMINLOG2);
#endif
    if (mag->n == 1 && !mag->ndx) {
        slabfree(virtslabtab, virthdrtab, ptr);
    } else if (mag->n) {
        magpush(mag, ptr);
        if (magempty(mag)) {
            slabfree(virtslabtab, virthdrtab, ptr);
            bkt = mag->bkt;
            maglk(bkt);
            if (mag->prev) {
                mag->prev->next = mag->next;
            } else {
                _freehdrtab[bkt] = mag->next;
            }
            if (mag->next) {
                mag->next->prev = mag->prev;
            }
            magunlk(bkt);
        } else if (mag->ndx == mag->n - 1) {
//            slab = &virthdrtab[slabnum(ptr)];
//            bkt = slabgetbkt(slab);
            bkt = mag->bkt;
            mag->prev = NULL;
            maglk(bkt);
            if (_freehdrtab[bkt]) {
                _freehdrtab[bkt]->prev = mag;
            }
            mag->next = _freehdrtab[bkt];
            _freehdrtab[bkt] = mag;
            magunlk(bkt);
        }
    }
#if (MEMTEST)
    magdiag();
#endif

    return;
}

