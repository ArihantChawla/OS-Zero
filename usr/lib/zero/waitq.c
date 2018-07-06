#include <stddef.h>
#include <stdint.h>
#include <zero/cdefs.h>
#include <mach/asm.h>
#include <zero/mtx.h>
#include <zero/waitq.h>

THREADLOCAL struct waitq *thrwaitq;

static struct waitq     *waitqtab[WAITQ_MAX];
static struct waitqitem  waitqitemtab[WAITQ_MAX];
static volatile long     waitqlk;
static long              waitqmaxid;
static struct waitqitem *waitqstk;

#if (ZEROFMTX)
#define __waitqlk(lp)   fmtxlk(lp)
#define __waitqunlk(lp) fmtxunlk(lp)
#endif

static struct waitq *
waitqinit(struct waitq *wq)
{
    if (!wq) {
        wq = malloc(sizeof(struct waitq));
    }
    if (wq) {
#if (ZEROFMTX)
        wq->lk = FMTXINITVAL;
#endif
        wq->flg = 0;
        wq->status = WAITQ_STATUS_NONE;
        wq->event = WAITQ_NONE;
        wq->prev = WAITQ_NONE;
        wq->next = WAITQ_NONE;
        wq->data = NULL;
        condinit(&wq->cond);
        wq->signal = NULL;
        wq->wait = NULL;
        wq->poll = NULL;
    }

    return wq;
}

static long
waitqgetid(void)
{
    long waitq;

    if (!thrwaitq) {
        thrwaitq = waitqalloc();
    }
    waitq = thrwaitq->id;

    return waitq;
}

long
waitqgetnext(long waitq)
{
    struct waitq *wq;

    while (waitq != WAITQ_NONE) {
        wq = waitqtab[waitq];
        waitq = wq->next;
    }
    waitq = wq->id;

    return waitq;
}

struct waitq *
waitqget(void)
{
    struct waitq     *wq = NULL;
    struct waitqitem *item;

    __waitqlk(&waitqlk);
    item = waitqstk;
    if (item) {
        waitqstk = item->next;
    }
    __waitqunlk(&waitqlk);
    if (item) {
        wq = item->wq;
    } else if (waitqmaxid < WAITQ_MAX) {
        wq = waitqinit(NULL);
        __waitqlk(&waitqlk);
        wq->id = waitqmaxid++;
        __waitqunlk(&waitqlk);
    }

    return wq;
}

long
waitqgetsize(long waitq)
{
    struct waitq *wq;
    long          nwait = 0;

    do {
        wq = waitqtab[waitq];
        waitq = wq->next;
        nwait++;
    } while (waitq != WAITQ_NONE);

    return nwait;
}

#if 0
static void
waitqrel(struct waitq *wq)
{
    struct waitqitem *item;
    long              id;

    if (wq) {
        id = wq->id;
        item = &waitqitemtab[id];
        waitqinit(wq);
        item->q = wq;
        __waitqlk(&waitqlk);
        item->next = waitqstk;
        waitqstk = item;
        __waitqunlk(&waitqlk);
    }

    return;
}
#endif

void
waitqwakeup(long waitq)
{
    ;
}

