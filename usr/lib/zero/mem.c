#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>
#include <zero/cdefs.h>
#include <mach/param.h>
#include <mach/asm.h>
#include <zero/mem.h>
#include <zero/unix.h>
#define TABHASH_VAL_NONE   (~(uintptr_t)0)
#define TABHASH_FUNC(key)  (tmhash32(key))
#include <zero/tabhash.h>
//#define LFDEQ_DATA_T       struct memslab
//#include <mt/lfdeq.h>
#define LFQ_ITEM_T       struct memslab
#include <zero/lfq.h>

struct divu16              fastu16divu16tab[MEM_MAX_FAST_DIV];
static struct mem          g_mem;
static struct membuf       g_membuf;
static struct memglob      g_memglob;
THREADLOCAL struct memtls *g_memtls;

#define MEM_TLS_TRIES      8
#define MEM_GLOB_TRIES     32
#define memblkistls(sz)    ((sz) <= MEM_RUN_MAX_SIZE)

static void
meminit(void)
{
    fastu16divu16gentab(fastu16divu16tab, MEM_MAX_FAST_DIV);

    return;
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

/* try to get a previously mapped header or map a new one */
#define MEMGETHDR_NTRIES 32
static struct memslab *
memgethdr(size_t n)
{
    struct memslab *hdr = NULL;
#if (MEMGETHDR_NTRIES)
    long            ntry = MEMGETHDR_NTRIES;
#endif

    if (n < MEM_SLAB_TAB_ITEMS / 2) {
        do {
            while ((uinptr_t)g_membuf.hdrq & MEM_ADR_LK_BIT) {
                m_waitspin();
            }
            if (m_cmpsetbit((m_atomic_t *)&g_membuf.hdrq, MEM_ADR_LK_BIT_POS)) {
                hdr = (void *)(uintptr_t)g_membuf.hdrq & ~MEM_ADR_LK_BIT;
                if (!hdr) {
                    hdr = mapanon(-1, MEM_SLAB_HDR_SIZE);
                    if (hdr == MAP_FAILED) {
                        hdr = (void *)~(uintptr_t)hdr;
                    }
                }
            }
        } while (!hdr
#if (MEMGETHDR_NTRIES)
                 && --ntry
#endif
                );
    }

    return hdr;
}

static void **
memgetstk(void)
{
    void **stk = NULL;

    do {
        while ((uinptr_t)g_membuf.tabq & MEM_ADR_LK_BIT) {
            m_waitspin();
        }
        if (m_cmpsetbit((m_atomic_t *)&g_membuf.tabq, MEM_ADR_LK_BIT_POS)) {
            stk = (void *)(uintptr_t)g_membuf.tabq & ~MEM_ADR_LK_BIT;
            if (!hdr) {
                stk = mapanon(-1, MEM_BLK_TAB_SIZE);
                if (stk == MAP_FAILED) {
                    stk = (void *)~(uintptr_t)stk;
                }
            }
        }
        m_waitspin();
        } while (!stk
#if (MEMGETHDR_NTRIES)
                 && --ntry
#endif
                );
    }

    return stk;
}

static struct memslab *
memmapslab(size_t n)
{
    struct memslab  *slab = memgethdr(n);
    void           **stk;

    if (!slab) {

        return NULL;
    }
    if (n <= MEM_SLAB_TAB_ITEMS / 2) {
        slab->stk = slab->tab;
    } else {
        stk = memgetstk(void);
        if (!stk) {
            fprintf(stderr, "MEM: failed to allocate pointer table\n");

            return NULL;
        }
    }

    return slab;
}

static void
memunmapslab(struct memslab *slab)
{
    void **stk = slab->stk;

    if (slab) {
        if (stk != slab->tab) {
            unmapanon(stk, MEM_BLK_TAB_SIZE);
        }
        unmapanon(slab, MEM_SLAB_HDR_SIZE);
    }

    return;
}

static void
memfreeslab(struct memslab *slab)
{
    size_t pool = memgetpool(slab);
    size_t type = memgettype(slab);

    if (memistls(type)) {
        type &= ~MEM_TLS_SLAB_BIT;
        switch (type) {
            case MEM_BLK_SLAB:
                head = g_memtls.blk[pool];
                if ((head) && !head->next) {
                    head->prev = slab;
                    slab->next = head;
                    g_memtls.blk[pool] = slab;
                    slab = NULL;
                }

                break;
            case MEM_RUN_SLAB:
                head = g_memtls.run[pool];
                if ((head) && !head->next) {
                    head->prev = slab;
                    slab->next = head;
                    g_memtls.run[pool] = slab;
                    slab = NULL;
                }

                break;
            default:

                break;
        }
    }
    if (slab) {
        memunmapslab(slab);
    }

    return;
}

static void
memqueueslab(struct memslab *slab)
{
}

static struct memslab *
meminitslab(size_t pool, size_t n, size_t bsize, uintptr_t type)
{
    size_t           sz = n * bsize;
    struct memslab  *slab = memmapslab(n);
    uint8_t         *ptr;
    void           **tab;

    if (!slab) {
        fprintf(stderr, "MEM: failed to map slab header\n");

        exit(1);
    }
    ptr = mapanon(-1, sz);
    if (!ptr) {
        fprintf(stderr, "MEM: failed to map slab of %ld bytes\n", (long)sz);

        exit(1);
    }
    tab = slab->stk;
    slab->base = ptr;
    slab->info = memmkinfo(pool, type);
    slab->nblk = n;
    slab->bsz = bsize;
    slab->ndx = 0;
    while (n--) {
        *tab = ptr;
        tab++;
        ptr += bsize;
    }

    return slab;
}

/* allocate thread-local memory */
static void *
memgetblktls(size_t size)
{
    void            *ptr = NULL;
#if (MEM_TLS_TRIES)
    long             ntry = MEM_TLS_TRIES;
#endif
    struct memslab **slot;
    struct memslab  *slab;
    struct memslab  *head;
    size_t           pool;
    intptr_t         n;

    if (size <= MEM_BLK_MAX_SIZE) {
        _memcalcblkpool(size, pool);
        slot = &g_memtls->blk[pool];
    } else {
        _memcalcrunpool(size, pool);
        slot = &g_memtls->run[pool];
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
            if (size <= MEM_BLK_MAX_SIZE) {
                slab = meminitslab(pool,
                                   memnumblk(pool), memblksize(pool),
                                   MEM_BLK_SLAB);
            } else {
                slab = meminitslab(pool,
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
                tabhashadd(g_mem.hash, memblkid(ptr), (uintptr_t)slab);
            }
        }
        if (!ptr) {
            m_waitspin();
        }
    } while (!ptr
#if (MEM_TLS_TRIES)
             && --ntry
#endif
            );

    return ptr;
}

/* allocate process-global memory */
static void *
memgetblkglob(size_t size)
{
    void            *ptr = NULL;
#if (MEM_GLOB_TRIES)
    long             ntry = MEM_GLOB_TRIES;
#endif
    struct memslab **slot;
    struct memslab  *slab;
    struct memslab  *head;
    size_t           pool;
    intptr_t         n;

    if (size > MEM_MID_MAX_SIZE) {
        ptr = mapanon(-1, membigsize(size));
        if (ptr) {
            tabhashadd(g_mem.hash, memblkid(ptr), (uintptr_t)size);
        }
    } else {
        _memcalcmidpool(size, pool);
        slot = &g_memglob.mid[pool];
        do {
            while ((m_atomic_t)*slot & MEM_ADR_LK_BIT) {
                m_waitspin();
            }
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
                    slab = meminitslab(pool,
                                       memnummid(pool), memmidsize(pool),
                                       MEM_MID_SLAB);
                    if (slab) {
                        n = slab->nblk;
                        ptr = slab->base;
                        slab->ndx = 1;
                        if (n > 1) {
                            m_atomwrite((m_atomic_t *)slot, (m_atomic_t)slab);
                        }
                        tabhashadd(g_mem.hash, memblkid(ptr), (uintptr_t)slab);
                    }
                }
            }
            if (!ptr) {
                m_waitspin();
            }
        } while (!ptr
#if (MEM_GLOB_TRIES)
                 --ntry
#endif
                );
    }

    return ptr;
}

static void *
memgetblk(size_t size, size_t align, long zero)
{
    void   *ptr = NULL;
    size_t  sz = max(size, MEM_MIN_SIZE);
    size_t  aln = max(align, MEM_MIN_ALIGN);

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
        ptr = memgetblktls(sz);
    }
    if (!ptr) {
        ptr = memgetblkglob(sz);
    }
    if (zero) {
        memzeroblk(ptr, size);
    }
    ptr = memalignptr(ptr, aln);

    return ptr;
}

/* action functions for lock-free queue */

/* pop item from queue entry stack */
uintptr_t
mempopblk(LFQ_ITEM_T **itemptr)
{
    LFQ_ITEM_T *item = (void *)((uintptr_t)*itemptr & ~LFQ_ADR_LK_BIT);
    uintptr_t   uptr;
    m_atomic_t  n;
    m_atomic_t  ndx;

    n = item->nblk;
    ndx = m_fetchadd(&item->ndx, -1);
    ndx++;
    uptr = item->stk[ndx];
    if (ndx == n) {
        /* remove totally in-use slab from queue */
        if (*itemptr->next) {
            *itemptr->next->prev = NULL;
        }
        m_atomwrite((m_atomic_t *)itemptr, (m_atomic_t)*itemptr->next);
    }

    return uptr;
}

/* push item into queue entry stack */
void
mempushblk(LFQ_ITEM_T **itemptr)
{
    LFQ_ITEM_T *item = (void *)((uintptr_t)*itemptr & ~LFQ_ADR_LK_BIT);
    LFQ_ITEM_T *end = NULL;
    uintptr_t   uptr;
    size_t      pool = memgetbool(item);
    size_t      type = memgettype(item);
    m_atomic_t  n;
    m_atomic_t  ndx;

    ndx = m_atomadd(&item->ndx, -1);
    n = item->nblk;
    ndx--;
    item->stk[ndx] = uptr;
    if (ndx == n) {
        /* queue previously unqueued totally in-use slab at queue tail */
        if (memistls(type)) {
            type &= ~MEM_TLS_SLAB_BIT;
            switch (type) {
                case MEM_BLK_SLAB:
                    end = g_memtls.blk[pool];
                    item->next = end;
                    if (end) {
                        item->prev = end->prev;
                        item->next = end;
                        end->prev = item;
                    } else {
                        item->prev = NULL;
                        item->next = NULL;
                        g_memtls.blk[pool] = item;
                    }

                    break;
                case MEM_RUN_SLAB:
                    end = g_memtls.run[pool];
                    item->next = end;
                    if (end) {
                        item->prev = end->prev;
                        item->next = NULL;
                        end->prev = item;
                    } else {
                        item->prev = NULL;
                        item->next = NULL;
                        g_memtls.run[pool] = item;
                    }

                    break;
                default:

                    break;
            }
        } else {
            switch (type) {
                case MEM_MID_SLAB:
                    lfqaddtail(&g_memglob.mid[pool], item);

                    break;
                case MEM_BIG_SLAB:
                    memunmapslab(item);

                    break;
            }
        }
    } else if (!ndx && n > 1) {
        if (memistls(type)) {
            type &= ~MEM_TLS_SLAB_BIT;
            switch (type) {
                case MEM_BLK_SLAB:
                    if (item->next) {
                        item->next->prev = item->prev;
                    } else {
                        g_memtls.blk[pool] = NULL;
                    }
                    if (item->prev) {
                        item->prev->next = item->next;
                    }

                    break;
                case MEM_RUN_SLAB:
                    if (item->next) {
                        item->next->prev = item->prev;
                    } else {
                        g_memtls.run[pool] = NULL;
                    }
                    if (item->prev) {
                        item->prev->next = item->next;
                    }

                    break;
                default:

                    break;
            }
        } else {
            switch (type) {
                case MEM_MID_SLAB:
                    lfqrmitem(&g_memglob.mid[pool], item);

                    break;
                case MEM_BIG_SLAB:
                    memunmapslab(item);

                    break;
            }
        }
    } else if (!ndx) {
        memunmapslab(item);
    }

    return;
}

