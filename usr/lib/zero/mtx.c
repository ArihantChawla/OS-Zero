#include <stdlib.h>
#include <zero/thr.h>
#include <zero/mtx.h>
#include <zero/asm.h>
#include <zero/thr.h>

zeromtxatr *
zeroallocmtxatr(void)
{
    zeromtxatr *atr = malloc(sizeof(zeromtxatr));

    if (atr) {
        atr->flg = __ZEROMTXATR_DYNAMIC;
    }

    return atr;
}

int
zerofreemtxatr(zeromtxatr *atr)
{
    int ret = 0;
    
    if (atr->flg & __ZEROMTXATR_DYNAMIC) {
        free(atr);
    } else {
        ret = ZEROMTXATR_NOTDYNAMIC;
    }

    return ret;
}

int
zeroinitmtxatr(zeromtxatr *atr)
{
    long dynflg = atr->flg & __ZEROMTXATR_DYNAMIC;
    if (!atr) {

        return -1;
    }
    atr->flg |= __ZEROMTXATR_INIT | dynflg;

    return 0;
}
    
long
zerotrylkmtx(zeromtx *mtx)
{
    volatile long res = 0;
    long          thr;

    if (!(mtx->atr.flg & ZEROMTX_RECURSIVE)) {
        /* non-recursive mutex */
        res = mtxtrylk(&mtx->val);
    } else {
        /* recursive mutex */
        thr = thrid();
        if (mtx->val == thr) {
            m_atominc(&mtx->cnt);
        } else if (!m_cmpswap(&mtx->cnt, 0, 1)) {
            mtx->val = thr;
        } else {
            
            return 0;
        }
        res = thr;
        mtx->rec++;
    }

    return res;
}

void
zerolkmtx(zeromtx *mtx)
{
    volatile long res = -1;
    long          thr;

    if (!(mtx->atr.flg & ZEROMTX_RECURSIVE)) {
        /* non-recursive mutex */
        mtxlk(&mtx->val);
    } else {
        /* recursive mutex */
        thr = thrid();
        if (m_atominc(&mtx->cnt)) {
            if (mtx->val != thr) {
                do {
                    res = mtxtrylk(&mtx->lk);
                    if (!res) {
                        thryield();
                    }
                } while (!res);
            }
        }
        mtx->val = thr;
        mtx->rec++;
    }

    return;
}

void
zerounlkmtx(zeromtx *mtx)
{
    volatile long res;
    long          thr;
    
    if (!(mtx->atr.flg & ZEROMTX_RECURSIVE)) {
        /* non-recursive mutex */
        mtxunlk(&mtx->val);
    } else {
        /* recursive mutex */
        thr = thrid();
        if (mtx->val == thr) {
            mtx->rec--;
            if (!mtx->rec) {
                mtx->val = ZEROMTX_FREE;
            }
            res = m_atomdec(&mtx->cnt);
            if (res > 1) {
                if (!mtx->rec) {
                    mtxunlk(&mtx->lk);
                }
            }
        }
    }

    return;
}

