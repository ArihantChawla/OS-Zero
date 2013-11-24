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

#if (BUFPERDEV)
struct bufdev           devtab[BUFNDEV] ALIGNED(PAGESIZE);
#elif (BUFNIDBIT <= 48)
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
#if (BUFPERDEV)
static volatile long    bufdevlk;
struct bufdev          *bufdevq;
#endif

long
bufinit(void)
{
#if (BUFPERDEV)
    struct bufdev *dev;
#endif
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
#if (BUFPERDEV)
        n = BUFNDEV;
        mtxlk(&bufdevlk);
        while (n--) {
            dev = &devtab[n];
            dev->next = bufdevq;
            bufdevq = dev;
        }
#endif
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

#endif /* 0 */

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

#if (BUFPERDEV)

void
bufaddblk(struct bufblk *blk)
{
    struct bufdev   *dev;
    uint64_t         key = bufkey(blk->num);
#if (BUFNIDBIT <= 48)
    long             key1 = (key >> 32) & 0xffff;
    long             key2 = (key >> 16) & 0xffff;
    struct bufblk  **tab;
#endif
    struct bufblk   *ptr = NULL;

    dev = &devtab[blk->dev];
#if (BUFNIDBIT <= 48)
    mtxlk(&dev->buflktab[key1]);
    tab = dev->buftab[key1];
    if (!tab) {
        tab = kmalloc(65536 * sizeof(struct bufblk *));
        kbzero(tab, 65536 * sizeof(struct bufblk *));
        dev->buftab[key1] = tab;
    }
    if (tab) {
        ptr = tab[key2];
    } else {
        /* ERROR */
    }
#else
    mtxlk(&bufhashlktab[key]);
    ptr = dev->bufhashtab[key];
#endif
    blk->tabprev = NULL;
    if (ptr) {
        ptr->tabprev = blk;
    }
    blk->tabnext = ptr;
#if (BUFNIDBIT <= 48)
    tab[key2] = blk;
    mtxunlk(&dev->buflktab[key1]);
#else
    bufhashtab[key] = blk;
    mtxunlk(&bufhashlktab[key]);
#endif
    bufqlru(blk);

    return;
}

struct bufblk *
buffindblk(long devid, uint64_t num, long rel)
{
    struct bufdev   *dev;
    struct bufblk   *blk = NULL;
    uint64_t         key = bufkey(num);
#if (BUFNIDBIT <= 48)
    long             key1 = (key >> 32) & 0xffff;
    long             key2 = (key >> 16) & 0xffff;
    struct bufblk  **tab;
#endif
    struct bufblk   *ptr;

    dev = &devtab[devid];
#if (BUFNIDBIT <= 48)
    mtxlk(&dev->buflktab[key1]);
    tab = dev->buftab[key1];
    if (!tab) {
        tab = kmalloc(65536 * sizeof(struct bufblk *));
        if (tab) {
            kbzero(tab, 65536 * sizeof(struct bufblk *));
        }
        dev->buftab[key1] = tab;
    }
    if (tab) {
        blk = tab[key2];
    }
#else
    mtxlk(&bufhashlktab[key]);
    blk = bufhashtab[key];
#endif
    while (blk) {
        if (blk->dev == devid && blk->num == num) {
            if (rel) {
                ptr = blk->tabprev;
                if (ptr) {
                    ptr->tabnext = blk->tabnext;
                } else {
#if (BUFNIDBIT <= 48)
                    tab[key2] = blk->tabnext;
#else
                    bufhashtab[key] = blk->tabnext;
#endif
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

#if (BUFNIDBIT <= 48)
    mtxunlk(&dev->buflktab[key1]);
#else
    mtxunlk(&bufhashlktab[key]);
#endif

    return blk;
}

#else /* !BUFPERDEV */

void
bufaddblk(struct bufblk *blk)
{
    uint64_t         key = bufkey(blk->num);
#if (BUFNIDBIT <= 48)
    long             key1 = (key >> 32) & 0xffff;
    long             key2 = (key >> 16) & 0xffff;
    struct bufblk  **tab;
#endif
    struct bufblk   *ptr = NULL;

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
    mtxunlk(&buflktab[key1]);
#else
    bufhashtab[key] = blk;
    mtxunlk(&bufhashlktab[key]);
#endif
    bufqlru(blk);

    return;
}

struct bufblk *
buffindblk(long devid, uint64_t num, long rel)
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
#else
    mtxlk(&bufhashlktab[key]);
    blk = bufhashtab[key];
#endif
    while (blk) {
        if (blk->dev == devid && blk->num == num) {
            if (rel) {
                ptr = blk->tabprev;
                if (ptr) {
                    ptr->tabnext = blk->tabnext;
                } else {
#if (BUFNIDBIT <= 48)
                    tab[key2] = blk->tabnext;
#else
                    bufhashtab[key] = blk->tabnext;
#endif
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

#if (BUFNIDBIT <= 48)
    mtxunlk(&buflktab[key1]);
#else
    mtxunlk(&bufhashlktab[key]);
#endif

    return blk;
}

#endif /* BUFPERDEV */

void
bufrel(long dev, uint64_t num, long flush)
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

