#include <stdint.h>
#include <zero/cdecl.h>
#include <zero/mtx.h>
#include <zero/waitq.h>

THREADLOCAL struct waitq *thrwaitq;

struct waitqitem {
    struct waitq *wq;
    struct waitq *next;
};

static struct waitq     *waitqtab[WAITQ_MAX];
static struct waitqitem  waitqitemtab[WAITQ_MAX];
static volatile long     waitqlk;
static long              waitqmaxid;
static struct waitqitem *waitqstk;

static struct waitq *
waitqinit(struct waitq *wq)
{
    if (!wq) {
        wq = malloc(1, sizeof(struct waitq));
    }
    if (wq) {
        wq->lk = MTXINITVAL;
        wq->flg = 0;
        wq->status = WAITQ_STATUS_NONE;
        wq->event = WAITQ_NONE;
        wq->prev = WAITQ_NONE;
        wq->next = WAITQ_NONE;
        wq->data = WAITQ_NONE;
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

    return id;
}

struct waitq *
waitqget(void)
{
    struct waitq     *wq = NULL;
    struct waitqitem *item;

    mtxlk(&waitqlk);
    item = waitqstk;
    if (item) {
        waitqstk = item->next;
    }
    mtxunlk(&waitqlk);
    if (item) {
        wq = item->wq;
    } else if (waitqmaxid < WAITQMAX) {
        wq = waitqinit(NULL);
        mtxlk(&waitqlk);
        wq->id = waitqmaxid++;
        mtxunlk(&waitqlk);
    }

    return wq;
}

long
waitqgetsize(long waitq)
{
    struct wq *wq;
    long       nwait = 0;

    while (waitq != WAITQ_NONE) {
        wq = waitqtab[waitq];
        waitq = wq->next;
        nwait++;
    }

    return nwait;
}

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
        mtxlk(&waitqlk);
        item->next = waitqstk;
        waitqstk = item;
        mtxunlk(&waitqlk);
    }

    return;
}

void
waitqwakeup(long waitq)
{
    ;
}

