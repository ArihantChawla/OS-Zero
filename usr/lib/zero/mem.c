#include <stddef.h>
#include <stdint.h>
#include <zero/cdefs.h>
#include <zero/param.h>
#include <zero/mem.h>

THREADLOCAL memtlsdata *g_memtlsdata;

#define MEM_SMALL_TRIES 16
#define MEM_RUN_TRIES   32

#define memblkisloc(sz, aln)                                            \
    ((sz) + (aln) - 1 <= MEM_MAX_RUN_PAGES * PAGESIZE)

/* allocate a block or run from thread-local buffer */
void *
memgetloc(size_t sz)
{
    void            *ptr = NULL;
    size_t           bkt;
    struct memslab **slot;
    struct memslab  *slab;
    size_t           ndx;
    size_t           n;
    long             ntry = MEM_SMALL_TRIES;

    if (sz <= MEM_MAX_SMALL_SIZE) {
        _memcalcsmallbkt(sz, bkt);
        slot = &g_memtlsdata->smalltab[bkt];
    } else {
        _memcalcrunbkt(sz, bkt);
        slot = &g_memtlsdata->runtab[bkt];
    }
    do {
        slab = *slot;
        if (slab) {
            ndx = m_fetchadd(&slab->ndx, 1);
            n = slab->n;
            if (ndx < n - 1) {
                ptr = slab->stk[ndx];
            } else if (ndx == n - 1) {
                && m_cmpswap(&slab->ndx, n, SIZE_MAX)) {
                if (slab->next) {
                    slab->next->prev = NULL;
                }
                ptr = slab->stk[n];
                *slab = slab->next;
                if (slab->prev) {
                    slab->prev->next = NULL;
                }
            }
        }
        if (!ptr) {
            m_spinwait();
        }
    } while (!ptr && (--ntry));

    return ptr;
}

/* allocate MEM_MAX_RUN..N bytes in a single global block */
void *
memgetbig(size_t sz)
{
    void            *ptr = NULL;
    size_t           bkt;
    struct memslab **slot;
    struct memslab  *slab;
    struct memslab  *next;
    size_t           ndx;
    size_t           n;

    _memcalcbigbkt(sz, bkt);
    slot = &g_mem.bigtab[bkt];
    do {
        if (!m_cmpsetbit((m_atomic_t *)slot, MEM_LK_BIT_OFS)) {
            slab = *slot;
            slab = (void *)((uintptr_t)slab & ~MEM_LK_BIT);
            if (slab) {
                ndx = m_fetchadd(&slab->ndx, 1);
                n = slab->n;
                if (ndx < n - 1) {
                    ptr = slab->stk[ndx];
                } else if (ndx == n - 1
                           && m_cmpswap(&slab->ndx, n, SIZE_MAX)) {
                    next = slab->next;
                    ptr = slab->stk[n];
                    if (next) {
                        next->prev = NULL;
                    }
                    if (slab->prev) {
                        slab->prev->next = NULL;
                    }
                    *slab = slab->next;
                } else {
                    m_fetchadd(&slab->ndx, -1);
                }
            }
            m_clrbit((m_atomic_t *)slot, MEM_LK_BIT_POS);
        }
        if (!ptr) {
            m_spinwait();
        }
    } while (!ptr && (--ntry));

    return ptr;
}

int
main(int argc, char *argv[])
{
}

