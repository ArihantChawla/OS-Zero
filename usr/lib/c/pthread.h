#if (PTHREAD)

#ifndef __PTHREAD_H__
#define __PTHREAD_H__

#if defined(linux)
#define __SIZEOF_PTHREAD_ATTR_T 56
typedef unsigned int pthread_key_t;
typedef union pthread_attr_t
{
    char __size[__SIZEOF_PTHREAD_ATTR_T];
    long __align;
} pthread_attr_t;
#endif /* defined(linux) */

extern int pthread_key_create(pthread_key_t *key, void (*destructor)(void *));
extern int pthread_atfork(void (*prepare)(void),
                          void (*parent)(void), void (*child)(void));

extern int pthread_setspecific(pthread_key_t key, const void *val);

#endif /* __PTHREAD_H__ */

#endif /* (PTHREAD) */

