#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <wpm/mem.h>
#include <zero/mtx.h>
#include <zero/trix.h>

#define WPMDEBUGMTX 1

static struct slab    memslabtab[1024 * 1024];
static volatile long  bktlktab[32];
static struct slab   *freetab[32];
static uint32_t       memvmbase;
static uint32_t       membrk;
static volatile long  memlk;
uint32_t             *mempagetab;

void
scanmem(void)
{
    uint32_t     bkt = MINBKT;
    struct slab *slab;
    uint32_t     n;

    while (bkt < 32) {
        n = 0;
        slab = freetab[bkt];
        while (slab) {
            fprintf(stderr, "%x - ", slabadr(slab));
            if (slab->bkt != bkt) {
                fprintf(stderr, "invalid bkt on free list: %d (%d)\n",
                        slab->bkt, bkt);

                exit(1);
            }
            n++;
            slab = slab->next;
        }
        fprintf(stderr, "%d: %d\n", bkt, n);
        bkt++;
    }
}

void
wpminitmem(uint32_t nbphys)
{
    uint32_t     nbvirt = MEMHWBASE - nbphys;
    uint32_t     adr = nbphys;
    uint32_t     bkt;
    struct slab *slab;

    mtxlk(&memlk, 1);
    memvmbase = nbphys;
    membrk = nbphys;
    if (!mempagetab) {
        mempagetab = calloc(1, 1024 * 1024 * sizeof(uint8_t *));
    }
    slab = &memslabtab[pagenum(nbphys)];
    while (nbvirt) {
        for (bkt = MINBKT ; bkt < 32 ; bkt++) {
            if (nbvirt & (1U << bkt)) {
                slab = &memslabtab[pagenum(adr)];
                slab->prev = NULL;
                slab->next = NULL;
                slab->bkt = bkt;
                freetab[bkt] = slab;
                adr += 1U << bkt;
                nbvirt &= ~(1U << bkt);
            }
        }
    }
    mtxunlk(&memlk, 1);

    return;
}

struct slab *
memsplitslab(uint32_t dest)
{
    uint32_t     bkt = dest;
    uint32_t     adr = 0;
    struct slab *slab1 = NULL;
    struct slab *slab2 = NULL;

    while (!adr && ++bkt < 32) {
        mtxlk(&bktlktab[bkt], 1);
        slab1 = freetab[bkt];
        if (slab1) {
            if (slab1->prev) {
                slab1->prev->next = slab1->next;
            }
            if (slab1->next) {
                slab1->next->prev = NULL;
            }
            freetab[bkt] = slab1->next;
            adr = slabadr(slab1) + (1U << dest);
            slab1->prev = NULL;
            slab1->next = NULL;
            slab1->bkt = dest;
        }
        mtxunlk(&bktlktab[bkt], 1);
    }
    if (adr) {
        for ( ; (dest < bkt) && (adr < MEMHWBASE) ; dest++) {
            slab2 = &memslabtab[pagenum(adr)];
            adr += 1U << dest;
            slab2->bkt = dest;
            slab2->free = 1;
            slab2->prev = NULL;
            slab2->next = freetab[dest];
            if (freetab[dest]) {
                freetab[dest]->prev = slab2;
            }
            freetab[dest] = slab2;
        }
    }

    return slab1;
}

void
memcombslab(struct slab *slab)
{
    uint32_t     bkt = slab->bkt;
    uint32_t     loop = 1;
    uint32_t     delta = 1U << (bkt - MINBKT);
    struct slab *prev;
    struct slab *next;
    uint32_t     queue = 0;

    while ((loop) && bkt < 32) {
        prev = slab - delta;
        next = slab + delta;
        mtxlk(&bktlktab[bkt], 1);
        if (slabadr(prev) >= MEMSIZE && (prev->free) && prev->bkt == bkt) {
            if (prev->prev) {
                prev->prev->next = prev->next;
            } else {
                freetab[bkt] = prev->next;
                if (prev->next) {
                    prev->next->prev = NULL;
                }
            }
            if (prev->next) {
                prev->next->prev = prev->prev;
            }
            slab = prev;
            slab->bkt = ++bkt;
            queue = 1;
        } else if ((next) && (next->free) && next->bkt == bkt) {
            if (next->prev) {
                next->prev->next = next->next;
            } else {
                freetab[bkt] = next->next;
                if (next->next) {
                    next->next->prev = NULL;
                }
            }
            if (next->next) {
                next->next->prev = next->prev;
            }
            slab->bkt = ++bkt;
            queue = 1;
        } else {
            loop = 0;
        }
        mtxunlk(&bktlktab[bkt], 1);
        delta <<= 1;
    }
    if (queue) {
        bkt = slab->bkt;
        mtxlk(&bktlktab[bkt], 1);
        slab->next = freetab[bkt];
        if (freetab[bkt]) {
            freetab[bkt]->prev = slab;
        }
        freetab[bkt] = slab;
        mtxunlk(&bktlktab[bkt], 1);
    }

    return;
}

uint32_t
mempalloc(uint32_t size)
{
    uint32_t     sz = powerof2(size) ? size : ceil2(max(size, 1U << MINBKT));
    uint32_t     n;
    uint32_t     adr = 0;
    uint8_t     *ptr = NULL;
    uint32_t     bkt = 0;
    uint32_t     num;
    struct slab *slab1;
    struct slab *slab2;

    tzero32(sz, bkt);
    mtxlk(&bktlktab[bkt], 1);
    slab1 = freetab[bkt];
    if (!slab1) {
        slab1 = memsplitslab(bkt);
    }
    if (slab1) {
        if (slab1->next) {
            slab1->next->prev = slab1->prev;
        }
        if (slab1->prev) {
            slab1->prev->next = slab1->next;
        }
        adr = slabadr(slab1);
        slab2 = slab1;
        slab1->bkt = bkt;
        n = 1U << (bkt - MINBKT);
        while (n--) {
            slab2->free = 0;
            slab2++;
        }
    }
    if  (adr) {
        ptr = malloc(1U << bkt);
        num = pagenum(adr);
        n = 1U << (bkt - MINBKT);
        while (n--) {
            mempagetab[num] = (uint32_t)ptr;
            num++;
            ptr += 1U << MINBKT;
        }
    }
    mtxunlk(&bktlktab[bkt], 1);

    return adr;
}

void
mempfree(uint32_t adr)
{
    uint32_t     num = pagenum(adr);
    void        *ptr;
    uint32_t     n;
    struct slab *slab;

    if (adr >= MEMSIZE && adr < MEMHWBASE) {
        mtxlk(&memlk, 1);
        slab = &memslabtab[num];
        ptr = (void *)mempagetab[num];
        if (ptr) {
            n = 1U << (slab->bkt - MINBKT);
            while (n--) {
                mempagetab[num] = (uint32_t)NULL;
                num++;
                slab->free = 1;
                slab++;
            }
            mtxunlk(&memlk, 1);
            memcombslab(slab);
            free(ptr);
        } else {
            fprintf(stderr, "invalid free %x\n", adr);

            exit(1);
        }
    }

    return;
}

