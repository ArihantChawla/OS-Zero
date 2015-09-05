#ifndef __ZERO_COND_H__
#define __ZERO_COND_H__

#if defined(ZEROMTX)
#include <zero/mtx.h>
#elif defined(PTHREAD)
#include <stddef.h>
#include <time.h>
#include <pthread.h>
#endif

#if defined(PTHREAD)

typedef pthread_cond_t zerocond;

#define condinit(cp)               pthread_cond_init(cp, NULL)
#define condwait(cp, mp)           pthread_cond_wait(cp, mp)
#define condwaittimed(cp, mp, tsp) pthread_cond_timedwait(cp, mp, tsp)
#define condsignal(cp)             pthread_cond_signal(cp)
#define condbcast(cp)              pthread_cond_broadcast(cp)
#define condfree(cp)               pthread_cond_destroy(cp)

#endif /* defined(PTHREAD) */

#endif /* __ZERO_COND_H__ */

