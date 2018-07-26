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
#if (!BUFMULTITAB) && (BUFNEWHASH)
#include <zero/hash.h>
#endif
//#include <kern/util.h>
#include <kern/util.h>
#include <kern/printf.h>
#include <kern/malloc.h>
#include <kern/mem/vm.h>
#include <kern/mem/slab.h>
#include <kern/mem/mag.h>
#include <kern/io/buf.h>

#undef  DEQ_ITEM_TYPE
#undef  DEQ_TYPE
#define DEQ_ITEM_TYPE struct bufblk
#define DEQ_TYPE      struct bufblk
#include <zero/deq.h>

#define bufadrtoid(ptr)                                                 \
    ((bufzone) ? ((uint8_t *)ptr - (uint8_t *)bufzone) >> BUFMINSIZELOG2 : NULL)

#if (BUFDYNALLOC)
static struct bufblk      *bufhdrtab;
#else
static struct bufblk       bufhdrtab[BUFNBLK] ALIGNED(PAGESIZE);
#endif
#if (BUFMULTITAB)
static m_atomic_t          buflktab[BUFNDEV] ALIGNED(PAGESIZE);
static void               *buftab[BUFNDEV] ALIGNED(PAGESIZE);
#elif (BUFNEWHASH)
static struct bufchain     bufhash[BUFNHASH];
#else
static m_atomic_t          buflktab[BUFNDEV] ALIGNED(PAGESIZE);
static void               *bufhash[BUFNDEV][BUFNHASH];
#endif
static struct bufdev       bufdevtab[BUFNDEV];
static struct bufblkqueue  buffreelist;
static struct bufblkqueue  buflruqueue;
static m_atomic_t          bufzonelk;
static void               *bufzone;
static m_ureg_t            bufnbyte;

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

#if (BUFDYNALLOC)
    sz = BUFNBLK * sizeof(struct bufblk);
    ptr = memalloc(sz, PAGEWIRED);
    if (!ptr) {
        kprintf("failed to allocate buffer cache headers\n");

        return 0;
    }
    bufhdrtab = ptr;
#endif
    /* allocate block I/O buffer cache */
    sz = BUFNBYTE;
    ptr = memalloc(sz, PAGEWIRED);
    if (!ptr) {
        do {
            sz >>= 1;
            ptr = memalloc(sz, PAGEWIRED);
        } while ((sz) >= BUFMINBYTES && !ptr);
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
    stats->nbuf = sz >> BUFMINSIZELOG2;
    stats->buf = ptr;
    stats->bufend = u8ptr + sz;
    vmmapseg((uint32_t)ptr, (uint32_t)ptr,
             (uint32_t)lim,
             PAGEBUF | PAGEPRES | PAGEWRITE | PAGEWIRED);
    stats->nphys += (sz >> PAGESIZELOG2);
    stats->nvirt += (sz >> PAGESIZELOG2);
    stats->nwire += (sz >> PAGESIZELOG2);
    stats->nbuf += (sz >> PAGESIZELOG2);
    kprintf("BUF: mapped buffer cache to %lx..%lx\n",
            (unsigned long)ptr, (unsigned long)(lim - 1));
    if (ptr) {
        /* zero buffer cache */
        kbzero(ptr, sz);
        /* initialise buffer headers */
        n = sz >> BUFMINSIZELOG2;
        blk = &bufhdrtab[0];
        blk->flg = BUFMINSIZELOG2;
        blk->data = u8ptr;
//            deqappend(blk, &buffreelist.head);
        u8ptr += BUFMINSIZE;
        prev = blk;
        blk++;
        while (--n) {
            prev->next = blk;
            blk->flg = BUFMINSIZELOG2;
            blk->data = u8ptr;
//            deqappend(blk, &buffreelist.head);
            u8ptr += BUFMINSIZE;
            blk++;
            prev = blk;
        }
        buffreelist.head = ptr;
        bufzone = ptr;
        bufnbyte = sz;
    }
#if 0
    if (ptr) {
        /* allocate and zero buffer cache */
        kbzero(ptr, sz);
        /* initialise buffer headers */
        n = sz >> BUFMINSIZELOG2;
        blk = &bufhdrtab[n - 1];
        u8ptr +=  sz;
        while (n--) {
            u8ptr -= BUFMINSIZE;
            blk->data = u8ptr;
            deqpush(blk, &buffreelist.head);
            blk--;
        }
        bufzone = ptr;
        bufnbyte = sz;
    }
#endif

    return 1;
}

void
bufwrite(struct bufblk *blk)
{
    /* TODO */
    ;
}

/* evict buffer; write back to disk */
struct bufblk *
bufevict(void)
{
    struct bufblk *blk = NULL;

    do {
        buflk(&buflruqueue.lk);
        blk = deqpop(&buflruqueue.head);
        bufunlk(&buflruqueue.lk);
        if (!blk) {
            /* TODO: wait for deqpop(&buflrudeq.head) */
        } else {
            if (blk->flg & BUFDIRTY) {
                bufwrite(blk);
            }
            bufclr(blk);
        }
    } while (!blk);

    return blk;
}

/* allocate buffer entry */
struct bufblk *
bufalloc(void)
{
    struct bufblk *blk = NULL;

    buflk(&buffreelist.lk);
    blk = deqpop(&buffreelist.head);
    bufunlk(&buffreelist.lk);
    if (!blk) {
        blk = bufevict();
    }

    return blk;
}

/*
 * multilevel table
 * ----------------
 * subtables are allocated and freed automatically
 * device ID bits 0..7 -> 8-bit buffer table index
 * block ID bits 38..47 -> 10-bit block table #1 index
 * block ID bits 28..37 -> 10-bit block table #2 index
 * block ID bits 16..27 -> 12-bit block table #3 index
 */

#if (!BUFMULTITAB)

void
bufaddblk(struct bufblk *blk)
{
#if (BUFNEWHASH)
    bufval_t         val = bufmkhashkey(blk->dev, blk->num);
    bufval_t         key = bufhash(val);
    struct bufblk   *buf;
    struct bufchain *chain = &bufhash[key];
#else
    buval_t          key = bufhash(blk->num);
    long             dkey = blk->dev & BUFDEVMASK;
    struct bufblk   *buf;
#endif

#if (BUFNEWHASH)
    buflk(&chain->lk);
    buf = chain->list;
    buf->tabprev = NULL;
    if (buf) {
        buf->tabprev = blk;
    }
    blk->tabnext = buf;
    chain->list = buf;
    bufunlk(&chain->lk);
#else
    buflk(&buflktab[dkey]);
    buf = bufhash[dkey][key];
    if (buf) {
        buf->tabprev = blk;
    }
    bufhash[dkey][key] = blk;
    bufunlk(&buflktab[dkey]);
#endif

    return;
}

/* look buffer up from buffer cache; dev is buffer-device ID, not system */
struct bufblk *
buffindblk(long dev, off_t num, long rel)
{
#if (BUFNEWHASH)
    bufval_t         val = bufmkhashkey(dev, num);
    bufval_t         key = bufhash(val);
    struct bufblk   *blk;
    struct bufchain *chain = &bufhash[key];
#else
    bufval_t         key = bufhash(num);
    long             dkey = dev & BUFDEVMASK;
    struct bufblk   *blk = NULL;
#endif

#if (BUFNEWHASH)
    buflk(&chain->lk);
    blk = chain->list;
    while ((blk) && blk->num != num) {
        blk = blk->tabnext;
    }
    if ((blk) && (rel)) {
        /* remove block from buffer hash chain */
        if (blk->tabprev) {
            blk->tabprev->tabnext = blk->tabnext;
        } else {
            chain->list = blk->tabnext;
        }
        if (blk->tabnext) {
            blk->tabnext->tabprev = blk->tabprev;
        }
    }

    return blk;
#else
    buflk(&buflktab[dkey]);
    blk = bufhash[dkey][key];
    while ((blk) && blk->num != num) {
        blk = blk->tabnext;
    }
    if ((blk) && (rel)) {
        /* remove block from buffer hash chain */
        if (blk->tabprev) {
            blk->tabprev->tabnext = blk->tabnext;
        } else {
            bufhash[dkey][key] = blk->tabnext;
        }
        if (blk->tabnext) {
            blk->tabnext->tabprev = blk->tabprev;
        }
    }
    bufunlk(&buflktab[dkey]);

    return blk;
#endif
}

#else /* BUFMULTITAB */

/* add block to buffer cache */
void
bufaddblk(struct bufblk *blk)
{
    bufval_t       key = bufkey(blk->num);
    long           dkey = blk->dev & BUFDEVMASK;
    long           bkey1 = (key >> BUFL1SHIFT) & BUFL1MASK;
    long           bkey2 = (key >> BUFL2SHIFT) & BUFL2MASK;
    long           bkey3 = (key >> BUFL3SHIFT) & BUFL3MASK;
    long           fail = 0;
    long           ndx;
    long           nref;
    struct bufblk *tab1;
    struct bufblk *tab2;
    struct bufblk *ptr = NULL;
    struct bufblk *btab;
    struct bufblk *bptr;
    void          *stk[3];

    buflk(&buflktab[dkey]);
    /* device table */
    tab1 = buftab[dkey];
    if (!tab1) {
        /* allocate */
        tab1 = kmalloc(BUFNL1ITEM * sizeof(struct bufblk));
        kbzero(tab1, BUFNL1ITEM * sizeof(struct bufblk));
        buftab[dkey] = tab1;
    }
    /* block table level #1 */
    if (tab1) {
        ptr = tab1;
        stk[0] = ptr;
        tab2 = ((struct bufblk **)tab1)[bkey1];
        if (!tab2) {
            /* allocate */
            tab2 = kmalloc(BUFNL2ITEM * sizeof(struct bufblk));
            kbzero(tab2, BUFNL2ITEM * sizeof(struct bufblk));
            ((struct bufblk **)tab1)[bkey1] = tab2;
        }
        if (tab2) {
            m_atominc(ptr->nref);
            /* block table level #2 */
            ptr = tab2;
            stk[1] = ptr;
            tab1 = ((struct bufblk **)tab2)[bkey2];
            if (!tab1) {
                tab1 = kmalloc(BUFNL3ITEM * sizeof(struct bufblk));
                kbzero(tab1, BUFNL3ITEM * sizeof(struct bufblk));
                ((struct bufblk **)tab2)[bkey2] = tab1;
            }
            if (tab1) {
                m_atominc(ptr->nref);
                ptr = tab1;
                stk[2] = ptr;
                /* block table level #3 */
                btab = ((struct bufblk **)tab1)[bkey3];
                if (btab) {
                    m_atominc(ptr->nref);
                    /* add to beginning of chain */
                    bptr = btab;
                    if (bptr) {
                        bptr->tabprev = blk;
                    }
                    blk->tabnext = bptr;
                    *((struct bufblk **)btab) = bptr;
                }
            } else {
                fail++;
            }
        } else {
            fail++;
        }
    } else {
        fail++;
    }
    if (fail) {
        ndx = 3;
        while (ndx--) {
            ptr = stk[ndx];
            if (ptr) {
                nref = ptr->nref;
                nref--;
                ptr->nref = nref;
                if (!nref) {
                    kfree(ptr);
                }
            }
        }
    }
    bufunlk(&buflktab[dkey]);
    if (!fail) {
        deqappend(blk, &buflruqueue.head);
    }

    return;
}

/* look buffer up from buffer cache */
struct bufblk *
buffindblk(long dev, off_t num, long rel)
{
    bufval_t       key = bufkey(num);
    struct bufblk *blk = NULL;
    long           dkey = dev & BUFDEVMASK;
    long           bkey1 = (key >> BUFL1SHIFT) & BUFL1MASK;
    long           bkey2 = (key >> BUFL2SHIFT) & BUFL2MASK;
    long           bkey3 = (key >> BUFL3SHIFT) & BUFL3MASK;
    long           ndx;
    long           nref;
    struct bufblk *ptr;
    struct bufblk *tab1;
    struct bufblk *tab2;
    struct bufblk *prev;
    struct bufblk *next;
    struct bufblk *stk[3];

    buflk(&buflktab[dkey]);
    /* device table */
    tab1 = buftab[dkey];
    if (tab1) {
        /* block table level #1 */
        stk[0] = tab1;
        tab2 = ((struct bufblk **)tab1)[bkey1];
        if (tab2) {
            /* block table level #2 */
            stk[1] = tab2;
            tab1 = ((struct bufblk **)tab2)[bkey2];
            if (tab1) {
                /* block table level #3 */
                stk[2] = tab1;
                blk = ((struct bufblk **)tab1)[bkey3];
                while (blk) {
                    /* scan chain */
                    if (blk->dev == dev && blk->num == num) {
                        if (rel) {
                            /* remove from chain */
                            prev = blk->tabprev;
                            next = blk->tabnext;
                            if (prev) {
                                prev->tabnext = blk->tabnext;
                            } else {
                                ((struct bufblk **)tab1)[bkey3] = next;
                            }
                            if (next) {
                                next->tabprev = prev;
                            }
                            /* deallocate empty subtables */
                            ndx = 3;
                            while (ndx--) {
                                ptr = stk[ndx];
                                nref = ptr->nref;
                                nref--;
                                ptr->nref = nref;
                                if (!nref) {
                                    kfree(ptr);
                                }
                            }
                        }

                        break;
                    }
                    blk = blk->tabnext;
                }
            }
        }
    }
    bufunlk(&buflktab[dkey]);

    return blk;
}

#endif

void
bufrel(long dev, bufval_t num, long flush)
{
    struct bufblk *blk = buffindblk(dev, num, 1);

    if (blk) {
#if 0
        if (flush) {
            bufwrite(blk);
        }
#endif
        buflk(&buffreelist.lk);
        deqpush(blk, &buffreelist.head);
        bufunlk(&buffreelist.lk);
    }

    return;
}

