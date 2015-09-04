/* REFERENCE: http://locklessinc.com/articles/barriers/ */

#include <zero/sys.h>
#include <zero/asm.h>
#include <zero/mtx.h>
#include <zero/cond.h>
#include <zero/bar.h>

void
barfree(zerobar *bar)
{
    mtxlk(&bar->lk);
    while (bar->num > BARFLAGBIT) {
        condwait(&bar->cond, &bar->lk);
    }
    mtxunlk(&bar->lk);
    condfree(&bar->cond);
}

void
barinit(zerobar *bar, unsigned long cnt)
{
    mtxinit(&bar->lk);
    condinit(&bar->cond);
    bar->cnt = cnt;
    bar->num = BARFLAGBIT;
}

long
barwait(zerobar *bar)
{
    mtxlk(&bar->lk);
    while (bar->num > BARFLAGBIT) {
        condwait(&bar->cond, &bar->lk);
    }
    if (bar->num == BARFLAGBIT) {
        bar->num = 0;
    }
    bar->num++;
    if (bar->num == bar->cnt) {
        bar->num += BARFLAGBIT - 1;
        condbcast(&bar->cond);
        mtxunlk(&bar->lk);

        return BARSERIALTHR;
    } else {
        while (bar->num < BARFLAGBIT) {
            condwait(&bar->cond, &bar->lk);
        }
        bar->num--;
        if (bar->num == BARFLAGBIT) {
            condbcast(&bar->cond);
        }
        mtxunlk(&bar->lk);

        return 0;
    }
}

void
barinitpool(zerobarpool *pool, unsigned long cnt)
{
    cnt--;
    pool->seq = 0;
    pool->cnt = 0;
    pool->nref = 1;
    pool->num = cnt;

    return;
}

void
barfreepool(zerobarpool *pool)
{
    m_fetchadd32(&pool->nref, -1);
    do {
        volatile long nref = m_atomread(pool->nref);

        if (!nref) {

            return;
        }
        syswait(&pool->nref, nref);
    } while (1);

    return;
}

long
barwaitpool(zerobarpool *pool)
{
    long ret;

    do {
        unsigned long seq = m_atomread(pool->seq);
        unsigned long cnt = m_fetchadd32(&pool->cnt, 1);

        if (cnt < pool->num) {
            while (m_atomread(pool->seq) == seq) {
                syswait(&pool->seq, seq);
            }
            ret = 0;

            break;
        }
        if (cnt == pool->num) {
            /* zero pool->cnt, increment pool->seq */
            m_membar();
            pool->rst = pool->seq + 1;
            m_membar();
            syswake(&pool->seq);
            ret = BARSERIALTHR;

            break;
        }
        /* we were too slow, so wait for barrier to be released */
        syswait(&pool->seq, seq);
    } while (1);
    if (m_fetchadd(&pool->nref, -1) == 1) {
        /* last one to wake up, wake destroying thread */
        syswait(&pool->nref, 1);
    }

    return ret;
}

