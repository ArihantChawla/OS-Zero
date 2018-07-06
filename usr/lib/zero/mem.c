#include <stddef.h>
#include <stdint.h>
#include <zero/cdefs.h>
#include <mach/param.h>
#include <mach/asm.h>
#include <zero/mem.h>
#define THASH_VAL_NONE  (~(uintptr_t)0)
#define THASH_FUNC(key) (tmhash32(key))
#include <zero/thash.h>

static struct mem          g_mem;
THREADLOCAL struct memtls *g_memtls;

#define MEM_TLS_TRIES     8
#define MEM_GLOB_TRIES    32
#define memistls(sz, aln) ((sz) + (aln) - 1 <= MEM_MAX_RUN_PAGES * PAGESIZE)

/* allocate thread-local memory */
void *
memgettls(size_t size, size_t align)
{
    void            *ptr = NULL;
    struct memslab **slot;
    struct memslab  *slab;
    size_t           pool;
    size_t           sz;
    size_t           ndx;
    size_t           n;
    long             ntry = MEM_TLS_TRIES;

    if (sz <= MEM_MAX_SMALL_SIZE) {
        _memcalcsmallpool(sz, pool);
        slot = g_memtls->smalltab[pool];
    } else {
        _memcalcrunpool(sz, pool);
        slot = g_memtls->runtab[pool];
    }
    do {
        slab = *slot;
        if (slab) {
            ndx = m_fetchadd(&slab->ndx, 1);
            n = slab->nblk;
            if (ndx < n - 1) {
                ptr = slab->stk[ndx];
            } else if (ndx == n - 1
                       && m_cmpswap(&slab->ndx, n, ~(uintptr_t)0)) {
                if (slab->next) {
                    slab->next->prev = NULL;
                }
                ptr = slab->stk[n];
                slab = slab->next;
                if (slab->prev) {
                    slab->prev->next = NULL;
                }
            }
        } else {
            if (sz <= MEM_MAX_MID_SIZE) {
                slab = memallocsmall(pool);
            } else {
                slab = memallocrun(pool);
            }
            if (slab) {
                ptr = mempopblk(slab);
                *slot = slab;
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
memgetglob(size_t size, size_t align)
{
    void            *ptr = NULL;
    struct memslab **slot;
    struct memslab  *slab;
    struct memslab  *next;
    size_t           pool;
    size_t           sz;
    size_t           ndx;
    size_t           nblk;
    long             ntry = MEM_GLOB_TRIES;

    if (sz <= MEM_MAX_MID_SIZE) {
        _memcalcmidpool(sz, pool);
        slot = &g_mem.midtab[pool];
    } else {
        _memcalcbigpool(sz, pool);
        slot = &g_mem.bigtab[pool];
    }
    do {
        if (!m_cmpsetbit((m_atomic_t *)slot, MEM_ADR_LK_BIT_POS)) {
            slab = *slot;
            slab = (void *)((uintptr_t)slab & ~MEM_ADR_LK_BIT);
            if (slab) {
                ndx = m_fetchadd(&slab->ndx, 1);
                nblk = slab->nblk;
                if (ndx < nblk - 1) {
                    ptr = slab->stk[ndx];
                } else if (ndx == nblk - 1
                           && m_cmpswap(&slab->ndx, n, ~(uintptr_t)0)) {
                    next = slab->next;
                    ptr = slab->stk[ndx];
                    if (next) {
                        next->prev = NULL;
                    }
                    if (slab->prev) {
                        slab->prev->next = NULL;
                    }
                    slab = slab->next;
                } else {
                    m_fetchadd(&slab->ndx, -1);
                }
                m_clrbit((m_atomic_t *)slot, MEM_ADR_LK_BIT_POS);
            } else {
                if (sz <= MEM_MAX_MID_SIZE) {
                    slab = memallocmid(pool);
                } else {
                    slab = memallocbig(pool);
                }
                if (slab) {
                    ptr = mempopblk(slab);
                    *slot = slab;
                }
            }
        }
        if (!ptr) {
            m_waitspin();
        }
    } while (!ptr && (--ntry));

    return ptr;
}

static void *
memgetblk(size_t size, size_t align, long zero)
{
    void   *ptr;
    size_t  sz = max(size, MEM_MIN_SIZE);
    size_t  aln = max(align, MEMMINALIGN);

    if (memblkistls(sz, aln)) {
        ptr = memgettls(sz, aln);
    } else {
        ptr = memgetglob(sz, aln);
    }
    if (zero) {
        memzeroblk(ptr, size);
    }
}

