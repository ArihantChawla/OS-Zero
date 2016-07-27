#include <zero/asm.h>
#include <zero/lwlk.h>
#include <zero/waitq.h>

static void
lwlklk(struct lwlk *lk, long wrlk)
{
    struct waitq *wq = waitqget();
    
    if (!lwlkasync(lk, wrlk)) {
        wq->wait();
    }

    return;
}

static void
lwlkunlkfair(struct lwlk *lk)
{
    struct waitq *wq;
    struct lwlk  *oldlk;
    struct lwlk  *newlk;

    do {
        oldlk = lk;
        newlk = lk;
        if (newlk->flg & LWLK_WRLK_BIT) {
            newlk->flg &= ~LWLK_WRLK_BIT;
        } else {
            newlk->nread--;
        }
        if (!((newlk->flg & LWLK_WRLK_BIT) || (newlk->nread > 0))) {
            waitq = waitqgetnext(newlk->waitq);
            newlk->waitq = waitq;
            if (waitq != WAITQ_NONE) {
                wq = waitqfind(waitq);
                if (wq->status != LWLK_WRLK) {
                    newlk->nread = waitqgetsize(waitq);
                } else {
                    newlk->flg |= LWLK_WRLK_BIT;
                }
            }
        }
    } while (!m_cmpswapptr(lk, oldlk, newlk));
    if (wq) {
        wq->next = WAITQ_NONE;
    }
    wq->signal();

    return;
}

static long
lwlklkasync(struct lwlk *lk, long wrlk)
{
    struct waitq *wq;
    struct lwlk   oldlk;
    struct lwlk  *newlk;

    wq = waitqget();
    do {
        oldlk = lk;
        newlk = lk;
        if (!wrlk && !(o->flg & LWLK_WRLK_BIT)
            && (oldlk->waitq == WAITQ_NONE
                || (oldlk->flg & LWLK_RDBIAS_BIT))) {
            /* read lock */
            old->nread++;
        } else if ((wrlk) &&
                   !((newlk->flg & LWLK_WRLK_BIT) || (newlk->nread > 0))) {
            /* write lock */
            newlk->flg |= LWLK_WRLK_BIT;
        } else {
            /* need to block */
            wq->status = LWLK_WRLK;
            wq->next = oldlk->waitq;
            newlk->waitq = waitq;
        }
    } while (!m_cmpswapptr(lk, oldlk, newlk));
    if (newlk->waitq == wq->id) {

        return LWLK_FAILURE;
    }
    
    return LWLK_SUCCESS;
}

