#ifndef __PTHREAD_H__
#define __PTHREAD_H__

#if !defined(__KERNEL__)

#if defined(PTHREAD) || defined(ZEROPTHREAD)

#include <features.h>

#if defined(linux) && !defined(ZEROPTHREAD)
#include <bits/pthreadtypes.h>
#endif /* defined(linux) */

#if defined(ZEROPTHREAD)

#if 0
#include <zero/thr.h>
#include <zero/mtx.h>
#endif
#include <bits/pthread.h>
/* mutexes */

#define PTHREAD_ONCE_INIT 0

/* spin locks */
int pthread_spin_destroy(pthread_spinlock_t *spin);
int pthread_spin_init(pthread_spinlock_t *spin, int pshared);
int pthread_spin_lock(pthread_spinlock_t *spin);
int pthread_spin_trylock(pthread_spinlock_t *spin);
int pthread_spin_unlock(pthread_spinlock_t *spin);

/* mutex locks */
int pthread_mutex_init(pthread_mutex_t *restrict mutex,
                       const pthread_mutexattr_t *restrict atr);
int pthread_mutex_destroy(pthread_mutex_t *mutex);
int pthread_mutex_lock(pthread_mutex_t *mutex);
int pthread_mutex_trylock(pthread_mutex_t *mutex);
int pthread_mutex_unlock(pthread_mutex_t *mutex);

/* miscellaneous routines */
int pthread_key_create(pthread_key_t *key, void (*destructor)(void *));
int pthread_atfork(void (*prepare)(void),
                   void (*parent)(void), void (*child)(void));
int pthread_setspecific(pthread_key_t key, const void *val);

#endif /* defined(ZEROPTHREAD) */

#endif /* defined(PTHREAD) || defined(ZEROPTHREAD) */

#endif /* !defined(__KERNEL__) */

#endif /* __PTHREAD_H__ */

