/*
 * buffer cache is allocated at-once (given size).
 * buffers are zeroed on-allocation, which forces them to be mapped and wired
 * to RAM.
 */

#define __KERNEL__ 1
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
#define bufqblk(blk)                                                    \
    listpush(&buffreelist, blk)
#define bufdeqblk(retpp)                                                \
    listdeq(&buffreelist, retpp)
#define bufpopblk(blk)                                                  \
    listqueue(&buffreelist, blk)
#define bufrmfree(blk)                                                  \
    listrm(&buffreelist, blk)
#define HASH_KEYTYPE long
#define HASH_TYPE    struct bufhash
#define HASH_ITEM    struct bufblkq
#define HASHPREV     hashprev
#define HASHNEXT     hashnext
#define HASH_TABSZ   65536
#define HASH_FUNC(bp) ((bp)->num & 0xffff)
#define HASH_CMP(bp1, bp2)                                              \
    ((bp1)->dev == (bp2)->dev && (bp1)->num == (bp2)->num)
#define HASH_ALLOC(n, sz) kwalloc((n) * (sz))
#include <zero/hash.h>

#if (!powerof2(HASH_TABSZ))
#error BUFNHASHITEM must be a power of two
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

static struct bufblk    *bufhashtab[BUFNHASHITEM] ALIGNED(PAGESIZE);
static volatile long     bufhashlktab[BUFNHASHITEM] ALIGNED(PAGESIZE);
//static volatile long     buflktab[BUFNBLK] ALIGNED(PAGESIZE);
static void             *bufstk[BUFNBLK] ALIGNED(PAGESIZE);
static struct bufblkq    buffreelist;
static volatile long     bufstklk;
static long              bufnstk;
static long              bufstknext;
static volatile long     bufzonelk;
static void             *bufzone;
//static uintptr_t         bufbrk;
static long              bufnbyte;

long
bufinit(void)
{
    long retval = 0;

    hashinit();
    kprintf("allocating %ld bytes of buffer cache\n", BUFNBYTE);
    bufzone = memalloc(BUFNBYTE, PAGEBUF | PAGEWIRED);
    if (bufzone) {
        retval = 1;
    }

    return retval;
}

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
        bufinit();
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

#if 0
void
bufaddblk(struct bufblk *blk)
{
    ;
}
#endif

void
bufaddblk(struct bufblk *blk)
{
    long           key = bufkey(blk->num);
    struct bufblk *tmp;
    struct bufblk *head;

    if (blk->freenext || blk->freeprev) {
        bufdeqblk(&tmp);
    }
    blk->freeprev = NULL;
    blk->freenext = NULL;
    blk->hashprev = NULL;
    blk->hashnext = NULL;
    mtxlk(&bufhashlktab[key]);
    head = bufhashtab[key];
    if (head) {
        head->hashprev = blk;
    }
    blk->hashnext = head;
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
                if (blk->hashprev) {
                    blk->hashprev->hashnext = blk->hashnext;
                } else {
                    bufhashtab[key] = blk->hashnext;
                }
                if (blk->hashnext) {
                    blk->hashnext->hashprev = blk->hashprev;
                }
            }

            break;
        }
        blk = blk->hashnext;
    }
    mtxunlk(&bufhashlktab[key]);

    return blk;
}

void
buffree(long dev, long num)
{
    struct bufblk *blk = devfindbuf(dev, num, 1);

    if (blk) {
        bufqblk(blk);
    }

    return;
}

