#ifndef __ZERO_SEM_H__
#define __ZERO_SEM_H__

#include <stddef.h>
#include <stdint.h>

#include <kern/conf.h>
#define ZEROSEMINITCNT NTHR
#if (__KERNEL__)
#include <kern/proc/thr.h>

#define ZEROSEMINITVAL { ZEROMTXINITVAL, ZEROSEMINITCNT, { NULL, NULL } }
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

static __inline__ zerosem *
semalloc(void)
{
    zerosem *sem = malloc(sizeof(zerosem));

    return sem;
}

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

#endif /* __ZERO_SEM_H__ */

