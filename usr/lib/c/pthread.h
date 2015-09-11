#include <features.h>

#if (PTHREAD)

#if defined(ZEROPTHREAD) && !(ZEROPTHREAD)
#undef ZEROPTHREAD
#endif

#ifndef __PTHREAD_H__
#define __PTHREAD_H__

#if defined(linux) && !defined(ZEROPTHREAD)
#include <bits/pthreadtypes.h>

int pthread_mutex_lock(pthread_mutex_t *mutex);
int pthread_mutex_trylock(pthread_mutex_t *mutex);
int pthread_mutex_unlock(pthread_mutex_t *mutex);

#endif /* defined(linux) */

#if (ZEROPTHREAD)

#include <bits/pthread.h>
/* mutexes */

#endif

#if !defined(__KERNEL__)

extern int pthread_key_create(pthread_key_t *key, void (*destructor)(void *));
extern int pthread_atfork(void (*prepare)(void),
                          void (*parent)(void), void (*child)(void));
extern int pthread_setspecific(pthread_key_t key, const void *val);

#endif /* !defined(__KERNEL__) */

#endif /* (PTHREAD) */

#endif /* __PTHREAD_H__ */

