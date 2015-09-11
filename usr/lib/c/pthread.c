#include <stdlib.h>
#include <errno.h>
#include <pthread.h>

#if defined(ZEROPTHREAD)

#define __PTHREAD_KEY_NHASHBIT 16
#define __PTHREAD_KEY_NHASH    (1UL << __PTHREAD_KEY_NHASHBIT)
#define __PTHREAD_KEY_NHASHTAB                                          \

struct {
    struct __pthread      *head;
    struct __pthread      *tail;
    struct __pthread_key **keyhash;
} g_pthread;

static int
__pthread_hashkey(struct __pthread_key *data)
{
    struct __pthread_key **ptr;
    unsigned long          hash = hashq128(&data->key,
                                           sizeof(pthread_key_t),
                                           __PTHREAD_KEY_NHASHBIT);

    if (!g_pthread.keyhash) {
        g_pthread.keyhash = calloc(__PTHREAD_KEY_NHASH,
                                   sizeof(struct __pthread_key *));
        if (!g_pthread.keyhash) {
            errno = ENOMEM;

            return -1;
        }
    }
    ptr = &g_pthread.keyhash[hash];
    data->next = *ptr;
    *ptr = data;

    return 0;
}

#endif /* defined(ZEROPTHREAD) */

