#include <stdlib.h>
#include <errno.h>
#if (PTHREAD)
#include <pthread.h>
#endif
#include <zero/cdecl.h>

static THREADLOCAL int   __errno;

#if (PTHREAD)
static pthread_key_t     __key;
static pthread_once_t    __once = PTHREAD_ONCE_INIT;
#endif

#if ((!defined(__STDC_VERSION__) || (__STDC_VERSION__ < 201112L)        \
      || defined(__STDC_NO_THREADS__))                                  \
     && !defined(__GNUC__))

static void
__errnoinit(void)
{
    pthread_key_create(&__key, free);

    return;
}

int *
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

int *
__errnoloc(void)
{
    int *ptr = &__errno;

    return ptr;
}

#endif

