#include <stdlib.h>
#include <errno.h>
//#include <pthread.h>

#if defined(__ZEROLIBC__)

#include <bits/pthread.h>

#define __PTHREAD_KEY_NHASHBIT 16
#define __PTHREAD_KEY_NHASH    (1UL << __PTHREAD_KEY_NHASHBIT)

#if (PTHREAD_KEY_SIZE == 4)
#define _pthreadhashkey(k) (tmhash32(k) & ((1U << __PTHREAD_KEY_NHASHBIT) - 1))
#elif (PTHREAD_KEY_SIZE == 8)
#define _pthreadhashkey(k) (tmhash64(k) & ((1UL << __PTHREAD_KEY_NHASHBIT) - 1))
#endif

struct {
    struct __pthread      *head;
    struct __pthread      *tail;
    struct __pthread_key **keyhash;
} g_pthread;

static int
__pthread_hashkey(struct __pthread_key *data)
{
    struct __pthread_key **ptr;
    unsigned long          hash = __pthreadhashkey(data->key);
    struct __pthread_key **tab;

    tab = g_pthread.keyhash;
    if (!tab) {
        tab = calloc(__PTHREAD_KEY_NHASH, sizeof(struct __pthread_key *));
        if (!tab) {
            errno = ENOMEM;

            return -1;
        }
        g_pthread.keyhash = tab;
    }
    ptr = &tab[hash];
    data->prev = NULL;
    data->next = *ptr;
    if (*ptr) {
        (*ptr)->prev = data;
    }
    *ptr = data;

    return 0;
}

#endif /* defined(ZEROPTHREAD) */

