#include <stdlib.h>
#include <errno.h>
#include <zero/cdefs.h>

#if (((!defined(__STDC_VERSION__) || (__STDC_VERSION__ < 201112L)       \
       || defined(__STDC_NO_THREADS__))                                 \
      && !defined(__GNUC__))                                            \
     && (PTHREAD))

#include <pthread.h>

static pthread_key_t   __key;
static pthread_once_t  __once = PTHREAD_ONCE_INIT;

static void
__errnoinit(void)
{
    pthread_key_create(&__key, free);

    return;
}

CONST int *
__errnoloc(void)
{
    int *ptr = NULL;

    pthread_once(&__once, __errnoinit);
    ptr = pthread_get_specific(__key);
    if (!ptr) {
        ptr = malloc(sizeof(int));
        pthread_setspecific(__key, ptr);
    }

    return ptr;
}

#else

static THREADLOCAL int __errno;

CONST int *
__errnoloc(void)
{
    int *ptr = &__errno;

    return ptr;
}

#endif

