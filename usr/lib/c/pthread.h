#ifndef __PTHREAD_H__
#define __PTHREAD_H__

extern int pthread_key_create(pthread_key_t *key, void (*destructor)(void *));
extern int pthread_atfork(void (*prepare)(void),
                          void (*parent)(void), void (*child)(void));

extern int pthread_setspecific(pthread_key_t key, const void *val);

#endif /* __PTHREAD_H__ */

