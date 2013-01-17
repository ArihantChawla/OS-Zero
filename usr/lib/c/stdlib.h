#ifndef __STDLIB_H__
#define __STDLIB_H__

#include <features.h>
#include <stddef.h>
#include <stdint.h>

#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1

#define RAND_MAX     0xffffffff

void exit(int status);

#define srand(useed) srand32(useed)
int    rand(void);

void * malloc(size_t size);
void * calloc(size_t n, size_t size);
void * realloc(void *ptr, size_t size);
void   free(void *ptr);
#if (_ISOC11_SOURCE)
void * aligned_alloc(size_t align, size_t size);
#endif

#if (_POSIX_C_SOURCE >= 200112L || _XOPEN_SOURCE >= 600)
int    posix_memalign(void **ret, size_t align, size_t size);
#endif /* POSIX || XOPEN */

#if ((_BSD_SOURCE)                                                      \
    || (_XOPEN_SOURCE >= 500 || ((_XOPEN_SOURCE) && (_XOPEN_SOURCE_EXTENDED))) \
    && !(_POSIX_C_SOURCE >= 200112L || _XOPEN_SOURCE >= 600))
void * valloc(size_t size);
#endif /* BSD || XOPEN || POSIX */

#if (_BSD_SOURCE)
void * reallocf(void *ptr, size_t size);
#endif /* BSD */

void * bsearch(const void *key, const void *base,
               size_t nitems, size_t itemsize,
               int (*cmp)(const void *, const void *));

void   qsort(void *const base, size_t nitems, size_t size,
             int (*cmp)(const void *, const void *));

#endif /* __STDLIB_H__ */
