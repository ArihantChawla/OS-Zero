#ifndef __PTHREAD_H__
#define __PTHREAD_H__

#if !defined(__KERNEL__)

#if defined(PTHREAD) || defined(ZEROPTHREAD)

#include <features.h>

#if defined(linux) && !defined(ZEROPTHREAD)
#include <bits/pthreadtypes.h>

int pthread_mutex_lock(pthread_mutex_t *mutex);
int pthread_mutex_trylock(pthread_mutex_t *mutex);
int pthread_mutex_unlock(pthread_mutex_t *mutex);

#endif /* defined(linux) */

#if defined(ZEROPTHREAD)

#if 0
#include <zero/thr.h>
#include <zero/mtx.h>
#endif
#include <bits/pthread.h>
/* mutexes */

#endif

extern int pthread_key_create(pthread_key_t *key, void (*destructor)(void *));
extern int pthread_atfork(void (*prepare)(void),
                          void (*parent)(void), void (*child)(void));
extern int pthread_setspecific(pthread_key_t key, const void *val);

#endif /* defined(PTHREAD) || defined(ZEROPTHREAD) */

#endif /* !defined(__KERNEL__) */

#endif /* __PTHREAD_H__ */

