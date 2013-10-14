/*
 * buffer cache is allocated at-once (given size).
 * buffers are zeroed on-allocation, which forces them to be mapped and wired
 * to RAM.
 */

#define __KERNEL__ 1
#include <stddef.h>
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
#define LISTPREV   freeprev
#define LISTNEXT   freenext
#include <zero/list.h>
#define bufqueueblk(blk)                                                \
    listpush(&buflruq, blk)
#define bufdeqblk(retpp)                                                \
    listdeq(&buflruq, retpp)
#define bufpopblk(blk)                                                    \
    listqueue(&buflruq, blk)
#define bufrmfree(blk)                                                  \
    listrm(&buflruq, blk)
#define HASH_KEYTYPE long
#define HASH_TYPE    struct bufhash
#define HASHPREV     hashprev
#define HASHNEXT     hashnext
#define HASH_TABSZ   65536
#define HASH_FUNC(bp) ((bp)->num & 0xffff)
#define HASH_CMP(bp1, bp2)                                              \
    ((bp1)->dev == (bp2)->dev && (bp1)->num == (bp2)->num)
#define HASH_ALLOC(n, sz) kwalloc((n) * (sz))
#include <zero/hash.h>

#if (!BUFMULTITAB)
#if (!powerof2(HASH_TABSZ))
#error BUFNHASHITEM must be a power of two
#endif
#endif

#define bufadrtoid(ptr)                                                 \
    ((bufzone) ? ((uint8_t *)ptr - (uint8_t *)bufzone) >> BUFSIZELOG2 : NULL)

#define bufempty()   (bufstknext == bufnstk)
//#define buffull()    (!bufstknext)
#define bufpop()     (bufstk[bufstknext++])
#define bufpush(ptr) (bufstk[--bufstknext] = (ptr))

#define buflk(adr)   mtxlk(&buflktab[bufadrtoid(adr)])
#define bufunlk(adr) mtxunlk(&buflktab[bufadrtoid(adr)])

/* TODO: stack for heap-based buffer allocation */

#if (!BUFMULTITAB)
static struct bufblkq   *bufhashtab[BUFNHASHITEM] ALIGNED(PAGESIZE);
#endif
static volatile long     buflktab[BUFNBLK] ALIGNED(PAGESIZE);
static void             *bufstk[BUFNBLK] ALIGNED(PAGESIZE);
static struct bufblkq    buflruq;
static volatile long     bufstklk;
static long              bufnstk;
static long              bufstknext;
static volatile long     bufzonelk;
static void             *bufzone;
//static uintptr_t         bufbrk;
static size_t            bufnbyte;

#if 0
void *
bufinit(void)
{
    hashinit(&bufhashtab);

    return bufhashtab;
}
#endif

/* flush nbuf buffers onto disks */
void *
bufevict(long nbuf)
{
    void *ptr = NULL;

    return ptr;
}

void *
bufalloc(void)
{
    void    *ptr = NULL;
    uint8_t *u8ptr;
    long     l;

    mtxlk(&bufzonelk);
    if (!bufzone) {
        bufzone = memalloc(BUFNBYTE, PAGEBUF | PAGEWIRED);
        if (bufzone) {
            /*
             * buffers are zeroed on allocation so we don't force them to be
             * mapped to ram with kbzero() yet.
             */
            u8ptr = bufzone;
            mtxlk(&bufstklk);
            for (l = 0 ; l < BUFNBLK ; l++) {
                bufstk[l] = u8ptr;
                u8ptr += BUFSIZE;
            }
            mtxunlk(&bufstklk);
            bufnbyte = BUFNBYTE;
        } else {

            return NULL;
        }
    }
    mtxunlk(&bufzonelk);
    do {
        mtxlk(&bufstklk);
        if (!bufempty()) {
            ptr = bufpop();
            mtxunlk(&bufstklk);
            kbzero(ptr, BUFSIZE);
        } else {
            bufevict(BUFNEVICT);
            mtxunlk(&bufstklk);
        }
    } while (!ptr);

    return ptr;
}

void
buffree(void *ptr)
{
    mtxlk(&bufstklk);
    bufpush(ptr);
    mtxunlk(&bufstklk);

    return;
}

#if (BUFMULTITAB)

void
devbufblk(struct devbuf *buf, struct devblk *blk)
{
    struct buftab *tab;
    struct buftab *ptr = NULL;
    struct bufblk *blk = NULL;
    long           num = blk->num;
    long           fail = 0;
    long           key0;
    long           key1;
    long           key2;
    long           key3;
    void          *pstk[BUFNKEY] = { NULL, NULL, NULL, NULL };

    key0 = bufkey0(num);
    key1 = bufkey1(num);
    key2 = bufkey2(num);
    key3 = bufkey3(num);
    mtxlk(&buf->lk);
    ptr = buf->tab.ptr;
    if (!ptr) {
        ptr = kmalloc(NLVL0BLK * sizeof(struct buftab));
        if (ptr) {
            kbzero(ptr, NLVL0BLK * sizeof(struct buftab));
        }
        pstk[0] = ptr;
    }
    if (ptr) {
        ptr = ptr[key0].ptr;
        if (!ptr) {
            ptr = kmalloc(NLVL1BLK * sizeof(struct buftab));
            if (ptr) {
                kbzero(ptr, NLVL1BLK * sizeof(struct buftab));
            }
            pstk[1] = ptr;
        }
    } else {
        fail = 1;
    }
    if (ptr) {
        ptr = ptr[key1].ptr;
        if (!ptr) {
            ptr = kmalloc(NLVL2BLK * sizeof(struct buftab));
            if (ptr) {
                kbzero(ptr, NLVL2BLK * sizeof(struct buftab));
            }
            pstk[2] = ptr;
        }
    } else {
        fail = 1;
    }
    if (ptr) {
        ptr = ptr[key2].ptr;
        if (!ptr) {
            ptr = kmalloc(NLVL3BLK * sizeof(struct buftab));
            if (ptr) {
                kbzero(ptr, NLVL3BLK * sizeof(struct buftab));
            }
            pstk[3] = ptr;
        }
    } else {
        fail = 1;
    }
    if (!fail) {
        if (pstk[0]) {
            buf->tab.nref++;
            buf->tab.ptr = pstk[0];
        }
        tab = buf->tab.ptr;
        if (pstk[1]) {
            tab[key0].nref++;
            tab[key0].ptr = pstk[1];
        }
        tab = tab[key0].ptr;
        if (pstk[2]) {
            tab[key1].nref++;
            tab[key1].ptr = pstk[2];
        }
        tab = tab[key1].ptr;
        if (pstk[3]) {
            tab[key2].nref++;
            tab[key2].ptr = pstk[3];
        }
        tab = tab[key2].ptr;
        tab = &tab[key3];
        tab->nref++;
        tab->ptr = blk;
    }
    mtxunlk(&buf->lk);
    
    return;
}

struct bufblk *
devfindbuf(struct devbuf *buf, long num, long rel)
{
    struct bufblk *blk = NULL;
    struct buftab *tab;
    long           key0;
    long           key1;
    long           key2;
    long           key3;

    key0 = bufkey0(num);
    key1 = bufkey1(num);
    key2 = bufkey2(num);
    key3 = bufkey3(num);
    mtxlk(&buf->lk);
    tab = buf->tab.ptr;
    if (tab) {
        tab = tab[key0].ptr;
        if (tab) {
            tab = tab[key1].ptr;
            if (tab) {
                tab = tab[key2].ptr;
                if (tab) {
                    blk = tab[key3].ptr;
                }
            }
        }
    }
    if (!rel) {
        mtxunlk(&buf->lk);
    }

    return blk;
}

void
devfreebuf(struct devbuf *buf, long num)
{
    long           key0 = bufkey0(num);
    long           key1 = bufkey1(num);
    long           key2 = bufkey2(num);
    long           key3 = bufkey3(num);
    void          *blk = devfindbuf(buf, num, 1);
    void          *ptr;
    struct buftab *tab;
    long           nref;
    long           val;
    void          *pstk[BUFNKEY] = { NULL, NULL, NULL, NULL };

    if (blk) {
        tab = buf->tab.ptr;
        nref = --tab->nref;
        if (!nref) {
            pstk[0] = tab;
            buf->tab.ptr = NULL;
        }
        tab = tab[key0].ptr;
        nref = --tab->nref;
        if (!nref) {
            pstk[1] = tab;
            tab[key0].ptr = NULL;
        }
        tab = tab[key1].ptr;
        nref = --tab->nref;
        if (!nref) {
            pstk[2] = tab;
            tab[key1].ptr = NULL;
        }
        tab = tab[key2].ptr;
        nref = --tab->nref;
        if (!nref) {
            pstk[3] = tab;
            tab[key2].ptr = NULL;
        }
        tab = &tab[key3];
        nref = --tab->nref;
        if (!nref) {
            for (val = 0 ; val < BUFNKEY ; val++) {
                ptr = pstk[val];
                if (ptr) {
                    kfree(ptr);
                }
            }
        }
    }
    mtxunlk(&buf->lk);
    bufqblk(&buflruq, blk);

    return;
}

#else /* !BUFMULTITAB */

#if 0
void
devbufblk(struct bufblk *blk)
{
    ;
}
#endif

#if 0
void
devbufblk(struct bufblk *blk)
{
    long           key = bufkey(blk->num);
    struct bufblk *head;

    blk->prev = NULL;
    blk->next = NULL;
    mtxlk(&bufhashlktab[key]);
    head = bufhashtab[key];
    if (head) {
        head->prev = blk;
    }
    blk->next = head;
    bufhashtab[key] = blk;
    mtxunlk(&bufhashlktab[key]);

    return;
}

struct bufblk *
devfindbuf(long dev, long num, long rel)
{
    long           key = bufkey(num);
    struct bufblk *blk;

    mtxlk(&bufhashlktab[key]);
    blk = bufhashtab[key];
    while (blk) {
        if (blk->dev == dev && blk->num == num) {
            if (rel) {
                if (blk->prev) {
                    blk->prev->next = blk->next;
                } else {
                    bufhashtab[key] = blk->next;
                }
                if (blk->next) {
                    blk->next->prev = blk->prev;
                }
            }

            break;
        }
        blk = blk->next;
    }
    mtxunlk(&bufhashlktab[key]);

    return blk;
}

void
devfreebuf(long dev, long num)
{
    struct bufblk *blk = devfindbuf(dev, num, 1);

    if (blk) {
        bufqblk(&buflruq, blk);
    }

    return;
}
#endif

#endif /* BUFMULTITAB */

