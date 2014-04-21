/*
 * buffer cache is allocated at-once (fixed size).
 * buffers are zeroed on-allocation, which forces them to be mapped to RAM
 * buffer address space is wired to physical memory
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
#define bufdeqlru(rpp)                                                  \
    listdeq(&buflruq, rpp)

#if (!powerof2(BUFNHASHITEM))
#error BUFNHASHITEM must be a power of two
#endif

#define bufadrtoid(ptr)                                                 \
    ((bufzone) ? ((uint8_t *)ptr - (uint8_t *)bufzone) >> BUFSIZELOG2 : NULL)

#if (BUFNIDBIT <= 48)
static struct bufblk  **buftab[BUFNDEV] ALIGNED(PAGESIZE);
static volatile long    buflktab[BUFNDEV] ALIGNED(PAGESIZE);
#else
static struct bufblk   *bufhashtab[BUFNHASHITEM] ALIGNED(PAGESIZE);
static volatile long    bufhashlktab[BUFNHASHITEM] ALIGNED(PAGESIZE);
#endif
static struct bufblk    bufhdrtab[BUFNBLK];
static struct bufblkq   buffreelist;
static struct bufblkq   buflruq;
static volatile long    bufzonelk;
static void            *bufzone;
static long             bufnbyte;

/* initialise buffer cache */
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
        kbzero(bufzone, BUFNBYTE);
        n = BUFNBLK;
        blk = bufhdrtab;
        u8ptr = bufzone;
        while (n--) {
            blk->data = u8ptr;
            bufpushfree(blk);
            u8ptr += BUFSIZE;
            blk++;
        }
        bufnbyte = BUFNBYTE;
        retval = 1;
    }

    return retval;
}

/* evict buffer; write back to disk */
struct bufblk *
bufevict(void)
{
    struct bufblk *blk = NULL;

    do {
        bufdeqlru(&blk);
    } while (!blk);
//    bufwrite(blk);
    
    return blk;
}

/* get free buffer entry */
struct bufblk *
bufgetfree(void)
{
    struct bufblk *blk = NULL;

    do {
        mtxlk(&buffreelist.lk);
        bufpopfree(&blk);
        mtxunlk(&buffreelist.lk);
        if (blk) {
            blk->listprev = NULL;
            blk->listnext = NULL;
            blk->tabprev = NULL;
            blk->tabnext = NULL;
        } else {
            blk = bufevict();
        }
    } while (!blk);
    
    return blk;
}

#define BUFNL1ITEM (1UL << 10)
#define BUFNL2ITEM (1UL << 10)
#define BUFNL3ITEM (1UL << 12)
/*
 * multilevel table
 * ----------------
 * device ID bits 0..7 -> buffer table index
 * block ID bits 38..47 -> block table #1 index
 * block ID bits 28..37 -> block table #2 index
 * block ID bits 16..27 -> block table #3 index
 */

/* add block to buffer cache */
void
bufaddblk(struct bufblk *blk)
{
    int64_t         key = bufkey(blk->num);
    long            dkey = blk->dev & 0xff;
    long            bkey1 = (key >> 38) & 0x3ff;
    long            bkey2 = (key >> 28) & 0x3ff;
    long            bkey3 = (key >> 16) & 0xfff;
    struct bufblk **tab1;
    struct bufblk **tab2;
    struct bufblk  *ptr = NULL;

    mtxlk(&buflktab[dkey]);
    /* device table */
    tab1 = buftab[dkey];
    if (!tab1) {
        /* allocate */
        tab1 = kmalloc(BUFNDEV * sizeof(struct bufblk *));
        kbzero(tab1, BUFNDEV * sizeof(struct bufblk *));
        buftab[dkey] = tab1;
    }
    /* block table level #1 */
    if (tab1) {
        tab2 = (struct bufblk **)(tab1[bkey1]);
    } else {
        /* allocate */
        tab2 = kmalloc(BUFNL1ITEM * sizeof(struct bufblk *));
        kbzero(tab2, BUFNL1ITEM * sizeof(struct bufblk *));
        tab1[bkey1] = (struct bufblk *)tab2;
    }
    if (tab2) {
        /* block table level #2 */
        tab1 = (struct bufblk **)(tab2[bkey2]);
        if (!tab1) {
            tab1 = kmalloc(BUFNL2ITEM * sizeof(struct bufblk *));
            kbzero(tab1, BUFNL2ITEM * sizeof(struct bufblk *));
            tab2[bkey2] = (struct bufblk *)tab1;
        }
        /* block table level #3 */
        tab2 = (struct bufblk **)(tab1[bkey3]);
        if (!tab2) {
            tab2 = kmalloc(BUFNL3ITEM * sizeof(struct bufblk *));
            kbzero(tab2, BUFNL3ITEM * sizeof(struct bufblk *));
            tab1[bkey3] = (struct bufblk *)tab2;
        }
        ptr = *tab2;
        /* add to beginning of chain */
        blk->tabprev = NULL;
        blk->tabnext = ptr;
        if (ptr) {
            ptr->tabprev = blk;
        }
        *tab2 = blk;
    }
    mtxunlk(&buflktab[dkey]);
    bufqlru(blk);

    return;
}

/* look buffer up from buffer cache */
struct bufblk *
buffindblk(int64_t dev, int64_t num, long rel)
{
    int64_t         key = bufkey(num);
    struct bufblk  *blk = NULL;
    long            dkey = dev & 0xff;
    long            bkey1 = (key >> 38) & 0x3ff;
    long            bkey2 = (key >> 28) & 0x3ff;
    long            bkey3 = (key >> 16) & 0xfff;
    struct bufblk **tab1;
    struct bufblk **tab2;
    struct bufblk  *prev;
    struct bufblk  *next;

    mtxlk(&buflktab[dkey]);
    /* device table */
    tab1 = buftab[dkey];
    if (tab1) {
        /* block table level #1 */
        tab2 = (struct bufblk **)(tab1[bkey1]);
        if (tab2) {
            /* block table level #2 */
            tab1 = (struct bufblk **)(tab2[bkey2]);
            if (tab1) {
                /* block table level #3 */
                blk = tab1[bkey3];
                while (blk) {
                    /* scan chain */
                    if (blk->dev == dev && blk->num == num) {
                        if (rel) {
                            prev = blk->tabprev;
                            next = blk->tabnext;
                            if (prev) {
                                ptr->tabnext = blk->tabnext;
                            } else {
                                tab1[bkey3] = next;
                            }
                            if (next) {
                                next->tabprev = prev;
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
        if (flush) {
//            bufwrite(blk);
        }
        bufpushfree(blk);
    }

    return;
}

