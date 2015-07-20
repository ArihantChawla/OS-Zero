#ifndef __ZERO_SEM_H__
#define __ZERO_SEM_H__

#include <stddef.h>

#include <kern/conf.h>
#define ZEROSEMINITCNT NTHR
#if (__KERNEL__)
#include <kern/proc/thr.h>

#define LISTQ_TYPE     struct thrqueue
#define LIST_TYPE      struct thr
#endif /* __KERNEL__ */

#include <zero/mtx.h>
#include <zero/list.h>

struct zerosem {
    zeromtx       lk;
    volatile long cnt;
    LISTQ_TYPE    queue;
};

typedef struct zerosem zerosem;

static __inline__ void
seminit(zerosem *sem)
{
    void *null = NULL;
    
    sem->lk = ZEROMTXINITVAL;
    sem->cnt = ZEROSEMINITCNT;
    sem->queue.head = null;
    sem->queue.tail = null;

    return;
}

static __inline__ void
semdown(zerosem *sem)
{
    volatile long cnt;
    
    mtxlk(&sem->lk);
    cnt = sem->cnt;
    if (!cnt) {
#if (__KERNEL__)
        listqueue(&sem->queue, k_curthr);
        k_curthr->wchan = (uintptr_t)sem;
        thraddwait(k_curthr);#endif
#endif
        mtxunlk(&sem->lk);
        m_waitint();
    } else {
        cnt--;
        sem->cnt = cnt;
        mtxunlk(&sem->lk);
    }

    return;
}

static __inline__ void
semup(zerosem *sem)
{
    LIST_TYPE     *item;
    volatile long  cnt;
    
    mtxlk(&sem->lk);
    cnt = sem->cnt;
    if (sem->queue.head) {
        listpop(&sem->queue, item);
        mtxunlk(&sem->lk);
#if (__KERNEL__)
        thrwakeup((uintptr_t)item);
#endif
    } else {
        cnt++;
        sem->cnt = cnt;
        mtxunlk(&sem->lk);
    }

    return;
}

#endif /* __ZERO_SEM_H__ */

