/*
 * buffer cache is allocated at-once (fixed size).
 * buffers are zeroed on-allocation, which forces them to be mapped to RAM
 * buffer address space is wired to physical memory
 */

/*
 * events
 * ------
 * bufqlru()
 */

/* TODO: implement per-device buffers */

#define __KERNEL__ 1
#include <sys/types.h>
#include <zero/cdecl.h>
#include <zero/param.h>
#include <zero/mtx.h>
#include <zero/trix.h>
#include <kern/util.h>
#include <kern/mem.h>
#include <kern/conf.h>
#include <kern/io/buf.h>
#if defined(__x86_64__) || defined(__amd64__)
#include <kern/unit/x86-64/vm.h>
#include <kern/mem/slab64.h>
#elif defined(__i386__)
#include <kern/unit/ia32/vm.h>
#include <kern/mem/slab32.h>
#endif
#include <kern/mem/mag.h>

#define LIST_TYPE  struct bufblk
#define LIST_QTYPE struct bufblkq
#define LISTPREV   listprev
#define LISTNEXT   listnext
#include <zero/list.h>
#define bufpushfree(blk)                                                \
    listpush(&buffreelist, blk)
#define bufpopfree(blk)                                                 \
    listpop(&buffreelist, blk)
#define bufqlru(blk)                                                    \
    listpush(&buflruq, blk)
#define bufdeqlru(blk)                                                  \
    listdeq(&buflruq, blk)
#define bufdeq(blk)                                                     \
    listrm(buflruq, blk)

#define bufadrtoid(ptr)                                                 \
    ((bufzone) ? ((uint8_t *)ptr - (uint8_t *)bufzone) >> BUFSIZELOG2 : NULL)

static struct bufblk   bufhdrtab[BUFNBLK] ALIGNED(PAGESIZE);
static void           *buftab[BUFNDEV] ALIGNED(PAGESIZE);
static volatile long   buflktab[BUFNDEV] ALIGNED(PAGESIZE);
static struct bufblkq  buffreelist;
static struct bufblkq  buflruq;
static volatile long   bufzonelk;
static void           *bufzone;
static long            bufnbyte;

/* initialise buffer cache; called at boot time */
long
bufinit(void)
{
    long           retval = 0;
    uint8_t       *u8ptr;
    struct bufblk *blk;
    long           n;

    kprintf("allocating %ld bytes of buffer cache\n", BUFNBYTE);
    bufzone = memalloc(BUFNBYTE, PAGEWIRED);
    if (bufzone) {
        /* allocate buffer cache */
        kbzero(bufzone, BUFNBYTE);
        /* initialise buffer headers */
        n = BUFNBLK;
        blk = &bufhdrtab[n - 1];
        u8ptr = bufzone;
        while (n--) {
            blk->data = u8ptr;
            bufpushfree(blk);
            u8ptr += BUFSIZE;
            blk--;
        }
        bufnbyte = BUFNBYTE;
        retval = 1;
    }

    return retval;
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
        bufdeqlru(&blk);
        if (!blk) {
            /* TODO: wait for bufqlru() */
        } else {
            if (blk->status & BUFMUSTWRITE) {
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

    mtxlk(&buffreelist.lk);
    bufpopfree(&blk);
    mtxunlk(&buffreelist.lk);
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

/* add block to buffer cache */
void
bufaddblk(struct bufblk *blk)
{
    int64_t        key = bufkey(blk->num);
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

    mtxlk(&buflktab[dkey]);
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
            ptr->nref++;
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
                ptr->nref++;
                ptr = tab1;
                stk[2] = ptr;
                /* block table level #3 */
                btab = ((struct bufblk **)tab1)[bkey3];
                if (btab) {
                    ptr->nref++;
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
    mtxunlk(&buflktab[dkey]);
    if (!fail) {
        bufqlru(blk);
    }
    
    return;
}

/* look buffer up from buffer cache */
struct bufblk *
buffindblk(dev_t dev, off_t num, long rel)
{
    int64_t        key = bufkey(num);
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

    mtxlk(&buflktab[dkey]);
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
    mtxunlk(&buflktab[dkey]);

    return blk;
}

void
bufrel(long dev, int64_t num, long flush)
{
    struct bufblk *blk = buffindblk(dev, num, 1);

    if (blk) {
#if 0
        if (flush) {
            bufwrite(blk);
        }
#endif
        bufpushfree(blk);
    }

    return;
}

