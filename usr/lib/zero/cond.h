#ifndef __ZERO_COND_H__
#define __ZERO_COND_H__

#include <zero/mtx.h>
#include <zero/thr.h>
#if defined(PTHREAD)
#include <stddef.h>
#include <time.h>
#include <pthread.h>
#endif

#if defined(PTHREAD) && !defined(ZEROCOND)

typedef pthread_cond_t zerocond;

#define condinit(cp)               pthread_cond_init(cp, NULL)
#define condwait(cp, mp)           pthread_cond_wait(cp, mp)
#define condwaittimed(cp, mp, tsp) pthread_cond_timedwait(cp, mp, tsp)
#define condsignal(cp)             pthread_cond_signal(cp)
#define condbcast(cp)              pthread_cond_broadcast(cp)
#define condfree(cp)               pthread_cond_destroy(cp)

#elif defined(ZEROCOND)

#define ZEROCOND_INITIALIZER { ZEROTHRQUEUE_INITIALIZER }
typedef struct {
    zerothrqueue queue;
} zerocond;

#endif

void condinit(zerocond *cond);
long condsignal(zerocond *cond);
long condsignal2(zerocond *cond, long nthr);
long condbroadcast(zerocond *cond);
#if (ZEROMTX)
long condwait(zerocond *cond, zeromtx *mtx);
long condwaitabstime(zerocond *cond, zeromtx *mtx,
                     const struct timespec *absts);
#elif (ZEROFMTX)
long condwait(zerocond *cond, zerofmtx *mtx);
long condwaitabstime(zerocond *cond, zerofmtx *mtx,
                     const struct timespec *absts);
#endif

#endif /* __ZERO_COND_H__ */

