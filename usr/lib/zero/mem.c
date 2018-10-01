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
#define LFDEQ_DATA_T       struct memslab
#include <mt/lfdeq.h>

struct divu16              fastu16divu16tab[MEM_MAX_FAST_DIV];
static struct mem          g_mem;
THREADLOCAL struct memtls *g_memtls;

#define MEM_TLS_TRIES      8
#define MEM_GLOB_TRIES     32
#define memblkistls(sz)    ((sz) <= MEM_RUN_MAX_SIZE)

static struct memslab * memmapslab(size_t pool, size_t n,
                                   size_t blksize, uintptr_t type);

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

    if (size <= MEM_BLK_MAX_SIZE) {
        _memcalcblkpool(size, pool);
        slot = &g_memtls->blktab[pool];
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
            if (size <= MEM_BLK_MAX_SIZE) {
                slab = memmapslab(pool,
                                  memnumblk(pool), memblksize(pool),
                                  MEM_BLK_SLAB);
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
                tabhashadd(g_mem.hash, memblkid(ptr), (uintptr_t)slab);
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

    if (size > MEM_MID_MAX_SIZE) {
        ptr = mapanon(-1, membigsize(size));
        if (ptr) {
            tabhashadd(g_mem.hash, memblkid(ptr), (uintptr_t)size);
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
                        tabhashadd(g_mem.hash, memblkid(ptr), (uintptr_t)slab);
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
        ptr = memgettls(sz);
    }
    if (!ptr) {
        ptr = memgetglob(sz);
    }
    if (zero) {
        memzeroblk(ptr, size);
    }
    ptr = memalignptr(ptr, aln);

    return ptr;
}

struct memslab *
meminitslab(size_t blksize, size_t n)
{
    struct memslab  *slab = mapanon(-1, sizeof(struct memslab));
    void           **stk;

    if (n <= MEM_SLAB_TAB_ITEMS / 2) {
        slab->stk = slab->tab;
    } else {
        stk = mapanon(-1, 2 * n * sizeof(void *));
        if (!stk) {
            unmapanon(slab, sizeof(struct memslab));

            return NULL;
        }
        slab->stk = stk;
    }

    return slab;
}

static struct memslab *
memmapslab(size_t pool, size_t n, size_t blksize, uintptr_t type)
{
    size_t           sz = n * blksize;
    struct memslab  *slab = meminitslab(blksize, n);
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
    slab->pool = pool;
    slab->nblk = n;
    slab->bsz = blksize;
    slab->ndx = 0;
    slab->info = type;
    while (n--) {
        *tab = ptr;
        tab++;
        ptr += blksize;
    }

    return slab;
}

