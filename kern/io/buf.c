/*
 * buffer cache is allocated at-once (fixed size).
 * buffers are zeroed on-allocation, which forces them to be mapped to RAM
 * buffer address space is wired to physical memory
 */

/* TODO: implement per-device buffers */

#define BUFSYNCQ   1

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
static struct bufblk  **buftab[BUFNTABITEM] ALIGNED(PAGESIZE);
static volatile long    buflktab[BUFNTABITEM] ALIGNED(PAGESIZE);
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
#if (BUFSYNCQ)
/* kept in sort by dev and num */
volatile long           bufsynclk;
static struct bufblk   *bufsyncq;
#endif

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

#if 0

void *
kgetdev(dev)
{
    return NULL;
}

void
devseek(long dev, off_t ofs)
{
    struct kdev *kdev = kgetdev(dev);

    kdev->seek(kdev, ofs);
}

static __inline__ void
bufseekdev(long dev, uint64_t num)
{
    devseek(dev, num * BUFSIZE);
}

void
bufwrite(struct bufblk *blk)
{
    bufseekdev(blk->dev, blk->num);
    bufwriteblk(blk->dev, blk->data);

    return;
}

#endif

#if (BUFSYNCQ)

void
bufaddsync(struct bufblk *blk)
{
    struct bufblk *item;
    struct bufblk *last = NULL;

    if (blk) {
        blk->listprev = NULL;
        blk->listnext = NULL;
        mtxlk(&bufsynclk);
        item = bufsyncq;
        if (item) {
            while ((item) && blk->dev < item->dev) {
                last = item;
                item = item->listnext;
            }
            while ((item) && blk->num < item->num) {
                last = item;
                item = item->listnext;
            }
            blk->listprev = last;
            blk->listnext = last->listnext;
            last->listnext = blk;
        } else {
            bufsyncq = blk;
        }
        mtxunlk(&bufsynclk);
    }
}

struct bufblk *
bufsync(void)
{
    struct bufblk *blk;
    struct bufblk *ret;

    mtxlk(&bufsynclk);
    blk = bufsyncq;
    ret = blk;
    while (blk) {
//        bufwrite(blk);
        blk = blk->listnext;
    }
    mtxunlk(&bufsynclk);

    return ret;
}

#endif

struct bufblk *
bufevict(void)
{
    struct bufblk *blk = NULL;
#if (BUFSYNCQ)
    long           n = 8;
#endif
    
#if (BUFSYNCQ)
    do {
        bufdeqlru(&blk);
        if (blk) {
            bufaddsync(blk);
            n--;
        }
    } while (n > 0);
    blk = bufsync();
#else
    do {
        bufdeqlru(&blk);
    } while (!blk);
//    bufwrite(blk);
#endif
    
    return blk;
}

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

void
bufaddblk(long dev, uint64_t num, void *data)
{
    struct bufblk   *blk;
    uint64_t         key = bufkey(num);
#if (BUFNIDBIT <= 48)
    long             key1 = (key >> 32) & 0xffff;
    long             key2 = (key >> 16) & 0xffff;
    struct bufblk  **tab;
#endif
    struct bufblk   *ptr;

    blk = bufgetfree();
    blk->dev = dev;
    blk->num = num;
    blk->data = data;
#if (BUFNIDBIT <= 48)
    mtxlk(&buflktab[key1]);
    tab = buftab[key1];
    if (!tab) {
        tab = kmalloc(65536 * sizeof(struct bufblk *));
        kbzero(tab, 65536 * sizeof(struct bufblk *));
        buftab[key1] = tab;
    }
    if (tab) {
        ptr = tab[key2];
    } else {
        /* ERROR */
    }
    mtxunlk(&buflktab[key1]);
#else
    mtxlk(&bufhashlktab[key]);
    ptr = bufhashtab[key];
#endif
    blk->tabprev = NULL;
    if (ptr) {
        ptr->tabprev = blk;
    }
    blk->tabnext = ptr;
#if (BUFNIDBIT <= 48)
    tab[key2] = blk;
#else
    bufhashtab[key] = blk;
#endif
    mtxunlk(&bufhashlktab[key]);
#endif
    bufqlru(blk);

    return;
}

struct bufblk *
buffind(long devid, uint64_t num, long rel)
{
    uint64_t         key = bufkey(num);
    struct bufblk   *blk = NULL;
#if (BUFNIDBIT <= 48)
    long             key1 = (key >> 32) & 0xffff;
    long             key2 = (key >> 16) & 0xffff;
    struct bufblk  **tab;
#endif
    struct bufblk   *ptr;

#if (BUFNIDBIT <= 48)
    mtxlk(&buflktab[key1]);
    tab = buftab[key1];
    if (!tab) {
        tab = kmalloc(65536 * sizeof(struct bufblk *));
        if (tab) {
            kbzero(tab, 65536 * sizeof(struct bufblk *));
        }
        buftab[key1] = tab;
    }
    if (tab) {
        blk = tab[key2];
    }
    mtxunlk(&buflktab[key1]);
#else
    mtxlk(&bufhashlktab[key]);
    blk = bufhashtab[key];
#endif
    while (blk) {
        if (blk->dev == dev && blk->num == num) {
            if (rel) {
                ptr = blk->tabprev;
                if (ptr) {
                    ptr->tabnext = blk->tabnext;
                } else {
                    bufhashtab[key] = blk->tabnext;
                }
                ptr = blk->tabnext;
                if (ptr) {
                    ptr->tabprev = blk->tabprev;
                }
            }

            break;
        }
        blk = blk->tabnext;
    }
#if (BUFNIDBIT > 48)
    mtxunlk(&bufhashlktab[key]);
#endif

    return blk;
}

void
bufrel(long dev, uint64_t num, long flush)
{
    struct bufblk *blk = buffind(dev, num, 1);

    if (blk) {
        if (flush) {
//            bufwrite(blk);
        }
        bufpushfree(blk);
    }

    return;
}

