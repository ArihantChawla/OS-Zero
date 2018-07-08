#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>
#include <zero/cdefs.h>
#include <mach/param.h>
#include <mach/asm.h>
#include <zero/mem.h>
#include <zero/unix.h>
#define THASH_VAL_NONE     (~(uintptr_t)0)
#define THASH_FUNC(key)    (tmhash32(key))
#include <zero/thash.h>
#define LFDEQ_DATA_T       struct memslab
#include <zero/lfdeq.h>

struct divu16              fastu16divu16tab[MEM_MAX_FAST_DIV];
static struct mem          g_mem;
THREADLOCAL struct memtls *g_memtls;

#define MEM_TLS_TRIES      8
#define MEM_GLOB_TRIES     32
#define memblkistls(sz)    ((sz) <= MEM_MAX_RUN_SIZE)

struct memslab * memmapslab(size_t pool, size_t n, size_t bsz, uint8_t type);

void
meminit(void)
{
    fastu16divu16gentab(fastu16divu16tab, MEM_MAX_FAST_DIV);

    return;
}

/* allocate thread-local memory */
void *
memgettls(size_t size)
{
    void            *ptr = NULL;
    long             ntry = MEM_TLS_TRIES;
    struct memslab **slot;
    struct memslab  *slab;
    struct memslab  *head;
    size_t           pool;
    intptr_t         n;

    if (size <= MEM_MAX_SMALL_SIZE) {
        _memcalcsmallpool(size, pool);
        slot = &g_memtls->smalltab[pool];
    } else {
        _memcalcrunpool(size, pool);
        slot = &g_memtls->runtab[pool];
    }
    do {
        slab = *slot;
        if (slab) {
            head = NULL;
            ptr = mempopblk(slab, &head);
            if (head) {
                head->prev = NULL;
                m_atomwrite((m_atomic_t *)slot, (m_atomic_t)head);
            }
        } else {
            if (size <= MEM_MAX_SMALL_SIZE) {
                slab = memmapslab(pool,
                                  memnumsmall(pool), memsmallsize(pool),
                                  MEM_SMALL_SLAB);
            } else {
                slab = memmapslab(pool,
                                  memnumrun(pool), memrunsize(pool),
                                  MEM_RUN_SLAB);
            }
            if (slab) {
                n = slab->nblk;
                ptr = slab->base;
                slab->ndx = 1;
                if (n > 1) {
                    m_atomwrite((m_atomic_t *)slot, (m_atomic_t)slab);
                }
                thashadd(g_mem.hash, memblkid(ptr), (uintptr_t)slab);
            }
        }
        if (!ptr) {
            m_waitspin();
        }
    } while (!ptr && (--ntry));

    return ptr;
}

/* allocate process-global memory */
void *
memgetglob(size_t size)
{
    void            *ptr = NULL;
    long             ntry = MEM_GLOB_TRIES;
    struct memslab **slot;
    struct memslab  *slab;
    struct memslab  *head;
    size_t           pool;
    intptr_t         n;

    if (size > MEM_MAX_MID_SIZE) {
        ptr = mapanon(-1, membigsize(size));
        if (ptr) {
            thashadd(g_mem.hash, memblkid(ptr), (uintptr_t)size);
        }
    } else {
        _memcalcmidpool(size, pool);
        slot = &g_mem.midtab[pool];
        do {
            if (!m_cmpsetbit((m_atomic_t *)slot, MEM_ADR_LK_BIT_POS)) {
                slab = *slot;
                slab = (void *)((uintptr_t)slab & ~MEM_ADR_LK_BIT);
                if (slab) {
                    head = NULL;
                    ptr = mempopblk(slab, &head);
                    if (head) {
                        head->prev = NULL;
                        m_atomwrite((m_atomic_t *)slot, (m_atomic_t)head);
                    }
                } else {
                    slab = NULL;
                    slab = memmapslab(pool,
                                      memnummid(pool), memmidsize(pool),
                                      MEM_MID_SLAB);
                    if (slab) {
                        n = slab->nblk;
                        ptr = slab->base;
                        slab->ndx = 1;
                        if (n > 1) {
                            m_atomwrite((m_atomic_t *)slot, (m_atomic_t)slab);
                        }
                        thashadd(g_mem.hash, memblkid(ptr), (uintptr_t)slab);
                    }
                }
            }
            if (!ptr) {
                m_waitspin();
            }
        } while (!ptr && (--ntry));
    }

    return ptr;
}

static void
memzeroblk(void *ptr, size_t size)
{
    uintptr_t *uptr = ptr;
    uintptr_t  uval = 0;
    size_t     sz = rounduppow2(size, MEM_MIN_SIZE);
    size_t     n;
    size_t     nw;

#if (PTRSIZE == 4)
    n = sz >> 2;
#elif (PTRSIZE == 8)
    n = sz >> 3;
#endif
    while (n) {
        nw = min(n, 8);
        switch (nw) {
            case 8:
                uptr[7] = uval;
            case 7:
                uptr[6] = uval;
            case 6:
                uptr[5] = uval;
            case 5:
                uptr[4] = uval;
            case 4:
                uptr[3] = uval;
            case 3:
                uptr[2] = uval;
            case 2:
                uptr[1] = uval;
            case 1:
                uptr[0] = uval;
            case 0:
            default:

                break;
        }
        n -= nw;
    }

    return;
}

static void *
memgetblk(size_t size, size_t align, long zero)
{
    void   *ptr;
    size_t  sz = max(size, MEM_MIN_SIZE);
    size_t  aln = max(align, MEMMINALIGN);

    if (!(g_mem.flg & MEM_INIT_BIT)) {
        fmtxlk(&g_mem.lk);
        meminit();
        g_mem.flg |= MEM_INIT_BIT;
        fmtxunlk(&g_mem.lk);
    }
    if (aln > sz || aln > PAGESIZE) {
        sz--;
        sz += aln;
    }
    if (memblkistls(sz)) {
        ptr = memgettls(sz);
    } else {
        ptr = memgetglob(sz);
    }
    if (zero) {
        memzeroblk(ptr, size);
    }
    ptr = memalignptr(ptr, aln);

    return ptr;
}

struct memslab *
memmapslab(size_t pool, size_t n, size_t bsz, uint8_t type)
{
    struct memslab  *slab = mapanon(-1, MEM_SLAB_SIZE);
    uint8_t         *ptr = mapanon(-1, n * bsz);
    void           **pptr;

    if (!slab || !ptr) {
        fprintf(stderr, "MEM: failed to map slab header\n");

        exit(1);
    }
    pptr = slab->tab;
    slab->base = ptr;
    slab->pool = pool;
    slab->nblk = n;
    slab->bsz = bsz;
    slab->ndx = 0;
    slab->type = type;
    while (n--) {
        *pptr = ptr;
        pptr++;
        ptr += bsz;
    }

    return slab;
}

