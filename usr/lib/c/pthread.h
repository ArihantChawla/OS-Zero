#include <features.h>

#if (PTHREAD)

#ifndef __PTHREAD_H__
#define __PTHREAD_H__

#if defined(linux)
#include <bits/pthreadtypes.h>

int pthread_mutex_lock(pthread_mutex_t *mutex);
int pthread_mutex_trylock(pthread_mutex_t *mutex);
int pthread_mutex_unlock(pthread_mutex_t *mutex);

#if 0
#define __SIZEOF_PTHREAD_ATTR_T 56
typedef unsigned int pthread_key_t;
typedef union pthread_attr_t
{
    char __size[__SIZEOF_PTHREAD_ATTR_T];
    long __align;
} pthread_attr_t;
#endif
#endif /* defined(linux) */

#if !defined(__KERNEL__)

extern int pthread_key_create(pthread_key_t *key, void (*destructor)(void *));
extern int pthread_atfork(void (*prepare)(void),
                          void (*parent)(void), void (*child)(void));
extern int pthread_setspecific(pthread_key_t key, const void *val);

#endif /* !defined(__KERNEL__) */

#endif /* (PTHREAD) */

#endif /* __PTHREAD_H__ */

