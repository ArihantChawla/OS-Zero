#ifndef __PTHREAD_H__
#define __PTHREAD_H__

#include <features.h>

#if defined(__GLIBC__) && !defined(ZEROPTHREAD)
#define PTHREAD_KEY_SIZE 4
#include <bits/pthreadtypes.h>
#endif /* defined(linux) */

#include <mach/param.h>
#include <bits/pthread.h>
/* mutexes */

#if !defined(PTHREAD_ONCE_INIT)
#define PTHREAD_ONCE_INIT 0
#endif

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

#include <bits/pthread.h>

#endif /* __PTHREAD_H__ */

