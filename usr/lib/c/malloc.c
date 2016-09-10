#include <stdint.h>
#include <errno.h>
#include <zero/cdefs.h>
#include <zero/param.h>
#include <zero/mem.h>
#include "_malloc.h"

extern struct mem g_mem;

static void *
_malloc(size_t size, size_t align, long flg)
{
    size_t  aln = max(align, MEMMINALIGN);
    size_t  sz = max(size, MEMMINBLK);
    size_t  bsz = (align <= sz) ? sz : sz + align;
    long    type = (memusesmallbuf(bsz)
                    ? MEMSMALLBLK
                    : (memusepagebuf(bsz)
                       ? MEMPAGEBLK
                       : MEMBIGBLK));
    long    slot = memcalcbufslot(sz, type);
    void   *ptr = memgetblk(slot, type);

    if (!ptr) {
#if defined(ENOMEM)
        errno = ENOMEM;
#endif
    } else if (flg & MALLOCZEROBIT) {
        memset(ptr, 0, size);
    }

    return ptr;
}

static void
_free(void *ptr)
{
    ;
}

void *
#if defined(__GNUC__)
__attribute__ ((alloc_size(2)))
__attribute__ ((assume_aligned(MEMMINALIGN)))
#endif
_realloc(void *ptr,
         size_t size,
         long rel)
{
    void          *retptr = NULL;
    struct membuf *buf = memfindbuf(ptr);
    void          *oldptr = membufgetptr(buf, ptr);
    size_t         sz = membufblksize(buf);

    if (!ptr) {
        retptr = _malloc(size, 0, 0);
    }
    if (retptr) {
        memcpy(retptr, oldptr, sz);
        _free(ptr);
        ptr = NULL;
    }
    ptr = retptr;
    if (((rel) && (ptr)) || (retptr != ptr)) {
        _free(ptr);
    }
#if (MEMDEBUG)
    assert(retptr != NULL);
#endif
    if (!retptr) {
#if defined(ENOMEM)
        errno = ENOMEM;
#endif
    }

    return retptr;
}

/* API FUNCTIONS */

void *
#if defined(__GNUC__)
__attribute__ ((alloc_size(1)))
__attribute__ ((assume_aligned(MEMMINALIGN)))
__attribute__ ((malloc))
#endif
malloc(size_t size)
{
    void *ptr = _malloc(size, 0, 0);

    return ptr;
}

void *
#if defined(__GNUC__)
__attribute__ ((alloc_size(1, 2)))
__attribute__ ((assume_aligned(MEMMINALIGN)))
__attribute__ ((malloc))
#endif
calloc(size_t n, size_t size)
{
    size_t  sz = n * size;
    void   *ptr = NULL;

    if (sz < n * size) {
        /* integer overflow */

        return NULL;
    }
    if (!sz) {
        ptr = _malloc(MEMMINBLK, 0, MALLOCZEROBIT);
    } else {
        ptr = _malloc(sz, 0, MALLOCZEROBIT);
    }

    return ptr;
}

void *
#if defined(__GNUC__)
__attribute__ ((alloc_size(2)))
__attribute__ ((assume_aligned(MEMMINALIGN)))
#endif
realloc(void *ptr, size_t size)
{
    void *retptr = NULL;

    if (!size && (ptr)) {
        _free(ptr);
    } else {
        retptr = _realloc(ptr, size, 0);
        if (retptr) {
            if (retptr != ptr) {
                _free(ptr);
            }
        }
    }
#if (MEMDEBUG)
    assert(retptr != NULL);
#endif

    return retptr;
}

void
free(void *ptr)
{
    _free(ptr);

    return;
}

#if defined(_BSD_SOURCE)
void *
#if defined(__GNUC__)
__attribute__ ((alloc_size(2)))
__attribute__ ((assume_aligned(MALLOCALIGNMENT)))
#endif
reallocf(void *ptr,
         size_t size)
{
    void *retptr;

    if (ptr) {
        retptr = _realloc(ptr, size, 1);
    } else if (size) {
        retptr = _malloc(size, 0, 0);
    } else {

        return NULL;
    }
#if (MEMDEBUG)
    assert(retptr != NULL);
#endif

    return retptr;
}
#endif

