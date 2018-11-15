/*
 * buffer cache is allocated at-once (fixed size).
 * buffers are zeroed on-allocation, which forces them to be mapped to RAM
 * buffer address space is wired to physical memory
 */

/*
 * events
 * ------
 * bufqueuelru()
 */

/* TODO: implement per-device buffers */

#include <kern/conf.h>
#include <sys/types.h>
#include <zero/cdefs.h>
#include <mach/param.h>
#include <zero/trix.h>
#if (BUFTKTLK)
#include <mt/tktlk.h>
#else
#include <mt/mtx.h>
#endif
#include <zero/hash.h>
//#include <kern/util.h>
#include <kern/util.h>
#include <kern/printf.h>
#include <kern/malloc.h>
#include <kern/mem/vm.h>
#include <kern/mem/slab.h>
#include <kern/mem/mag.h>
#include <kern/io/buf.h>

#if 0
#undef  DEQ_ITEM_TYPE
#undef  DEQ_TYPE
#define DEQ_ITEM_TYPE struct bufblk
#define DEQ_TYPE      struct bufblk
#include <zero/deq.h>
#endif

#define bufadrtoid(ptr)                                                 \
    ((bufzone) ? ((uint8_t *)ptr - (uint8_t *)bufzone) >> BUFBLKSHIFT : NULL)

static struct bufblk *bufhash[BUFHASHITEMS];
static struct bufblk  buffreelist;
static struct bufblk  buflruqueue;
static m_atomic_t     bufzonelk;
static uint8_t       *bufzone;
static m_ureg_t       bufnbyte;
static struct bufblk *bufhdrtab;

/* allocate and initialise buffer cache; called at boot time */
long
ioinitbuf(void)
{
    struct vmpagestat *stats = &k_physmem.pagestat;
    uint8_t           *u8ptr;
    void              *ptr = NULL;
    struct bufblk     *blk;
    struct bufblk     *prev;
    long               n;
    long               sz;
    m_ureg_t      lim;

    sz = BUFBLKS * BUFBLKHDRSIZE;
    ptr = memalloc(sz, PAGEWIRED);
    if (!ptr) {
        kprintf("failed to allocate buffer cache headers\n");

        return 0;
    }
    bufhdrtab = ptr;
    /* allocate block I/O buffer cache */
    sz = BUFCACHESIZE;
    ptr = memalloc(sz, PAGEWIRED);
    if (!ptr) {
        do {
            sz >>= 1;
            ptr = memalloc(sz, PAGEWIRED);
        } while ((sz) >= BUFMINCACHESIZE && !ptr);
    }
    if (!ptr) {
        kprintf("failed to allocate buffer cache\n");

        return 0;
    }
#if (__KERNEL__)
    kprintf("BUF: reserved %lu bytes for buffer cache\n", sz);
#endif
    u8ptr = ptr;
    lim = (m_ureg_t)(u8ptr + sz);
    stats->nbuf = sz >> BUFBLKSHIFT;
    stats->buf = ptr;
    stats->bufend = u8ptr + sz;
    vmmapseg((uint32_t)ptr, (uint32_t)ptr,
             (uint32_t)lim,
             PAGEPRES | PAGEWRITE | PAGEWIRED);
    stats->nphys += sz >> BUFBLKSHIFT;
    stats->nvirt += sz >> BUFBLKSHIFT;
    stats->nwire += sz >> BUFBLKSHIFT;
    stats->nbuf += sz >> BUFBLKSHIFT;
    kprintf("BUF: mapped buffer cache to %lx..%lx\n",
            (unsigned long)ptr, (unsigned long)(lim - 1));
    if (ptr) {
        /* zero buffer cache */
        kbzero(ptr, sz);
        /* initialise buffer headers */
        n = sz >> BUFBLKSHIFT;
        blk = &bufhdrtab[0];
        blk->flg = BUFBLKSHIFT;
        blk->data = u8ptr;
        //            deqappend(blk, &buffreelist.next);
        u8ptr += BUFBLKSIZE;
        prev = blk;
        blk++;
        while (--n) {
            prev->next = blk;
            blk->flg = BUFBLKSHIFT;
            blk->data = u8ptr;
            //            deqappend(blk, &buffreelist.next);
            u8ptr += BUFBLKSIZE;
            blk++;
            prev = blk;
        }
        buffreelist.next = ptr;
        bufzone = ptr;
        bufnbyte = sz;
    }

    return 1;
}

void
bufwrite(struct bufblk *blk)
{
    /* TODO */
    ;
}

struct bufblk *
bufpop(struct bufblk *queue)
{
    m_atomic_t     val = FMTXINITVAL;
    m_atomic_t     res;
    struct bufblk *blk;

    do {
        while (queue->prev) {
            m_waitspin();
        }
        res = m_cmpswap((m_atomic_t *)&queue->prev, (m_atomic_t)NULL, val);
    } while (!res);
    blk = queue->next;
    if (blk) {
        queue->next = blk->next;
    } else {
        queue->next = NULL;
    }
    m_atomwrite((m_atomic_t *)&queue->prev, NULL);

    return blk;
}

void
bufpush(struct bufblk *queue, struct bufblk *blk)
{
    m_atomic_t     val = FMTXINITVAL;
    m_atomic_t     res;
    struct bufblk *buf;

    do {
        while (queue->prev) {
            m_waitspin();
        }
        res = m_cmpswap((m_atomic_t *)&queue->prev, 0, val);
    } while (!res);
    buf = queue->next;
    if (buf) {
        buf->prev = blk;
    }
    queue->next = blk;
    m_atomwrite((m_atomic_t *)&queue->prev, NULL);

    return;
}

/* evict buffer; write back to disk */
struct bufblk *
bufevict(void)
{
    struct bufblk *blk = NULL;

    blk = bufpop(&buflruqueue);
    if (blk) {
        if (blk->flg & BUFDIRTY) {
            bufwrite(blk);
        }
        bufclr(blk);
        bufpush(&buffreelist, blk);
    }

    return blk;
}

/* allocate buffer entry */
struct bufblk *
bufget(void)
{
    struct bufblk *blk = NULL;

    blk = bufpop(&buffreelist);
    if (!blk) {
        blk = bufevict();
    }

    return blk;
}

void
bufaddblk(struct bufblk *blk)
{
    long           key = blk->id;
    long           res;
    uintptr_t      uptr;
    struct bufblk *buf;

    key = tmhash32(key) & (BUFHASHITEMS - 1);
    do {
        while ((uintptr_t)bufhash[key] & BUF_LK_BIT) {
            m_waitspin();
        }
        res = m_cmpsetbit((m_atomic_t *)&bufhash[key], BUF_LK_BIT_POS);
    } while (!res);
    uptr = (uintptr_t)bufhash[key];
    uptr &= ~BUF_LK_BIT;
    blk->xlist = uptr;
    bufaddref(blk);
    m_atomwrite((m_atomic_t *)&bufhash[key], blk);

    return;
}

struct bufblk *
buffindblk(long dev, bufid_t num, long rel)
{
    long           key = bufsetid(dev, num);
    long           res;
    uintptr_t      uptr;
    uintptr_t      xptr;
    struct bufblk *buf;
    struct bufblk *blk;
    struct bufblk *tmp;

    key = tmhash32(key) & (BUFHASHITEMS - 1);
    do {
        while ((uintptr_t)bufhash[key] & BUF_LK_BIT) {
            m_waitspin();
        }
        res = m_cmpsetbit((m_atomic_t *)&bufhash[key], BUF_LK_BIT_POS);
    } while (!res);
    uptr = (uintptr_t)bufhash[key];
    uptr &= ~BUF_LK_BIT;
    buf = NULL;
    blk = (void *)uptr;
    xptr = 0;
    while (blk) {
        uptr = blk->xlist;                      // uptr = buf ^ blk->next
        if (bufgetblknum(blk->id) == num) {
            if (rel) {
                bufdropref(blk);
                /* remove item we found */
                if (buf) {
                    /* remove non-head item from queue */
                    /* update buf, i.e. blk->prev */
                    xptr = buf->xlist;      // xptr = buf->prev ^ blk
                    uptr ^= (uintptr_t)buf; // uptr = blk->next
                    xptr ^= (uintptr_t)blk; // xptr = buf->prev
                    tmp = (void *)uptr;     // tmp = blk->next
                    uptr ^= xptr;           // uptr = buf->prev ^ blk->next
                    buf->xlist = uptr;      // buf->xlist = buf->prev ^ blk->next
                    if (tmp) {
                        /* not the last item in queue */
                        /* update blk->next */
                        uptr = tmp->xlist;      // uptr = blk ^ blk->next->next
                        uptr ^= (uintptr_t)blk; // uptr = blk->next->next
                        uptr ^= (uintptr_t)buf; // uptr = blk->prev ^ blk->next->next
                        tmp->xlist = uptr;      // blk->next->xlist = blk->prev ^ blk->next->next
                    }
                    /* release hash-chain bitlock */
                    m_clrbit((m_atomic_t *)&bufhash[key], BUF_LK_BIT_POS);
                } else {
                    /* remove head item from queue */
                    tmp = (void *)blk->xlist;   // blk->next
                    if (tmp) {
                        xptr = tmp->xlist;      // xptr = blk ^ blk->next->next
                        xptr ^= (uintptr_t)blk; // xptr = blk->next->next
                        tmp->xlist = xptr;
                    }
                    m_atomwrite((m_atomic_t *)&bufhash[key], tmp);
                }
            } else {
                m_clrbit((m_atomic_t *)&bufhash[key], BUF_LK_BIT_POS);
            }

            return blk;
        } else {
            uptr ^= (uintptr_t)buf;             // uptr = blk->next
            buf = blk;                          // buf = blk;
            blk = (void *)uptr;                 // blk = blk->next
        }
    }

    return blk;
}

