#include <features.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#if (MEMDEBUG)
#include <stdio.h>
//#include <crash.h>
#endif
#include <errno.h>
#include <malloc.h>
#include <zero/cdefs.h>
#include <zero/param.h>
#include <zero/mem.h>
#include <zero/mtx.h>
#include <zero/trix.h>
#include <zero/valgrind.h>
#include "_malloc.h"

extern THREADLOCAL volatile struct memtls *g_memtls;
extern struct mem                          g_mem;

static void *
_malloc(size_t size, size_t align, long flg)
{
    size_t  aln = max(align, MEMMINALIGN);
    size_t  sz = max(size, MEMMINBLK);
    size_t  asz = (aln <= PAGESIZE && aln <= sz) ? sz : sz + aln;
    long    type = (memusesmallbuf(asz)
                    ? MEMSMALLBUF
                    : (memusepagebuf(asz)
                       ? MEMPAGEBUF
                       : MEMBIGBUF));
    long    slot;
    void   *ptr;

    if (!g_memtls) {
        meminittls();
        if (!g_memtls) {
#if defined(ENOMEM)
            errno = ENOMEM;
#endif

            return NULL;
        }
    }
    if (type != MEMPAGEBUF) {
        memcalcslot(asz, slot);
    } else {
        memcalcpageslot(asz, slot);
    }
    ptr = memgetblk(slot, type, size, aln);
    if (!ptr) {
#if defined(ENOMEM)
        errno = ENOMEM;
#endif
    } else {
        VALGRINDALLOC(ptr, size, 0, flg & MALLOCZEROBIT);
        if (flg & MALLOCZEROBIT) {
            memset(ptr, 0, size);
        }
    }
#if (MEMDEBUG)
    crash(ptr != NULL);
#endif

    return ptr;
}

static void
_free(void *ptr)
{
    MEMADR_T       desc;
#if (MEMMULTITAB)
    struct membuf *buf;
#endif

    if (!g_memtls) {
        meminittls();
        if (!g_memtls) {

            exit(ENOMEM);
        }
    }
#if (MEMMULTITAB)
    memfindbuf(ptr, 1);
#else
    desc = membufop(ptr, MEMHASHDEL, NULL, 0);
    if (desc) {
        VALGRINDFREE(ptr);
    }
#endif

    return;
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
    MEMPTR_T       retptr = NULL;
#if (MEMMULTITAB)
    struct membuf *buf = (ptr) ? memfindbuf(ptr, 0) : NULL;
#else
    MEMADR_T       desc = 0;
    struct membuf *buf;
#endif
//    MEMPTR_T       oldptr = (buf) ? membufgetptr(buf, ptr) : NULL;
    MEMUWORD_T     type;
    MEMUWORD_T     slot;
    size_t         sz;

    if (!ptr) {
        retptr = _malloc(size, 0, 0);
        
        return retptr;
    } else if (!size) {
        free(ptr);
        
        return NULL;
    } else {
        desc = membufop(ptr, MEMHASHCHK, NULL, 0);
        buf = (struct membuf *)(desc & ~MEMPAGEINFOMASK);
        if (desc) {
            type = memgetbuftype(buf);
            slot = memgetbufslot(buf);
            sz = membufblksize(buf, type, slot);
            if (size <= sz) {
                
                return ptr;
            }
            retptr = _malloc(size, 0, 0);
            if (retptr) {
                memcpy(retptr, ptr, sz);
                _free(ptr);
                ptr = NULL;
            }
        }
        if ((rel) && (ptr)) {
            _free(ptr);
        }
#if (MEMDEBUG)
        crash(retptr != NULL);
#endif
        if (!retptr) {
#if defined(ENOMEM)
            errno = ENOMEM;
#endif
        }
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

    if (!sz) {
        sz++;
    } else if (sz < n * size) {
        /* integer overflow */

        return NULL;
    }
    ptr = _malloc(sz, 0, MALLOCZEROBIT);

    return ptr;
}

void *
#if defined(__GNUC__)
__attribute__ ((alloc_size(2)))
__attribute__ ((assume_aligned(MEMMINALIGN)))
#endif
realloc(void *ptr, size_t size)
{
    void *retptr = _realloc(ptr, size, 0);

#if (MEMDEBUG)
    crash(retptr != NULL);
#endif

    return retptr;
}

#if (_POSIX_C_SOURCE >= 200112L || _XOPEN_SOURCE >= 600)
int
#if defined(__GNUC__)
__attribute__ ((alloc_size(3)))
__attribute__ ((alloc_align(2)))
#endif
posix_memalign(void **ret,
               size_t align,
               size_t size)
{
    void *ptr = NULL;

    if (!powerof2(align) || (align & (sizeof(void *) - 1))) {
        *ret = NULL;

        return EINVAL;
    } else {
        ptr = _malloc(size, align, 0);
        if (!ptr) {
            *ret = NULL;
            
            return ENOMEM;
        }
    }
#if (MEMDEBUG)
    crash(ptr != NULL);
#endif
    *ret = ptr;

    return 0;
}
#endif

void
free(void *ptr)
{
    if (ptr) {
        _free(ptr);
    }

    return;
}

#if defined(_BSD_SOURCE)
void *
#if defined(__GNUC__)
__attribute__ ((alloc_size(2)))
__attribute__ ((assume_aligned(MEMMINALIGN)))
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
    crash(retptr != NULL);
#endif

    return retptr;
}
#endif

void *
#if defined(__GNUC__)
__attribute__ ((alloc_align(1)))
__attribute__ ((alloc_size(2)))
__attribute__ ((assume_aligned(MEMMINALIGN)))
__attribute__ ((malloc))
#endif
memalign(size_t align,
         size_t size)
{
    void   *ptr = NULL;

    if (!powerof2(align)) {
        errno = EINVAL;
    } else {
        ptr = _malloc(size, align, 0);
    }
#if (MEMDEBUG)
    crash(ptr != NULL);
#endif

    return ptr;
}

#if defined(_ISOC11_SOURCE)
#if defined(__GNUC__)
__attribute__ ((alloc_align(1)))
__attribute__ ((alloc_size(2)))
__attribute__ ((assume_aligned(MEMMINALIGN)))
__attribute__ ((malloc))
#endif
void *
aligned_alloc(size_t align,
              size_t size)
{
    void   *ptr = NULL;

    if (!powerof2(align) || (size & (align - 1))) {
        errno = EINVAL;
    } else {
        ptr = _malloc(size, align, 0);
    }
#if (MEMDEBUG)
    crash(ptr != NULL);
#endif

    return ptr;
}
#endif /* _ISOC11_SOURCE */

#if (defined(_BSD_SOURCE)                                                      \
     || (defined(_XOPEN_SOURCE) && _XOPEN_SOURCE >= 500                 \
         || (defined(_XOPEN_SOURCE) && defined(_XOPEN_SOURCE_EXTENDED))) \
     && !((defined(_POSIX_SOURCE) && _POSIX_C_SOURCE >= 200112L)        \
          || (defined(_XOPEN_SOURCE) && _XOPEN_SOURCE >= 600)))
void *
#if defined(__GNUC__)
__attribute__ ((alloc_size(1)))
__attribute__ ((assume_aligned(PAGESIZE)))
__attribute__ ((malloc))
#endif
valloc(size_t size)
{
    void *ptr;

    ptr = _malloc(size, PAGESIZE, 0);
#if (MEMDEBUG)
    crash(ptr != NULL);
#endif
    
    return ptr;
}
#endif

#if defined(_GNU_SOURCE)
void *
#if defined(__GNUC__)
__attribute__ ((alloc_size(1)))
__attribute__ ((assume_aligned(PAGESIZE)))
__attribute__ ((malloc))
#endif
pvalloc(size_t size)
{
    size_t  sz = rounduppow2(size, PAGESIZE);
    void   *ptr = _malloc(sz, PAGESIZE, 0);

#if (MEMDEBUG)
    crash(ptr != NULL);
#endif

    return ptr;
}
#endif

#if defined(_MSVC_SOURCE)

void *
#if defined(__GNUC__)
__attribute__ ((alloc_align(2)))
__attribute__ ((alloc_size(1)))
__attribute__ ((assume_aligned(MEMMINALIGN)))
__attribute__ ((malloc))
#endif
_aligned_malloc(size_t size,
                size_t align)
{
    void   *ptr = NULL;

    if (!powerof2(align)) {
        errno = EINVAL;
    } else {
        ptr = _malloc(size, align, 0);
    }
#if (MEMDEBUG)
    crash(ptr != NULL);
#endif

    return ptr;
}

void
_aligned_free(void *ptr)
{
    if (ptr) {
        _free(ptr);
    }

    return;
}

#endif /* _MSVC_SOURCE */

#if defined(_INTEL_SOURCE) && !defined(__GNUC__)

void *
#if defined(__GNUC__)
__attribute__ ((alloc_align(2)))
__attribute__ ((alloc_size(1)))
__attribute__ ((assume_aligned(MEMMINALIGN)))
__attribute__ ((malloc))
#endif
_mm_malloc(size_t size,
           size_t align)
{
    void   *ptr = NULL;

    if (!powerof2(align)) {
        errno = EINVAL;
    } else {
        ptr = _malloc(size, align, 0);
    }
#if (MEMDEBUG)
    crash(ptr != NULL);
#endif

    return ptr;
}

void
_mm_free(void *ptr)
{
    if (ptr) {
        _free(ptr);
    }

    return;
}

#endif /* _INTEL_SOURCE && !__GNUC__ */

void
cfree(void *ptr)
{
    if (ptr) {
        _free(ptr);
    }

    return;
}

size_t
malloc_usable_size(void *ptr)
{
#if (MEMMULTITAB)
    struct membuf *buf = (ptr) ? memfindbuf(ptr, 0) : NULL;
#else
    MEMADR_T       desc = ((ptr)
                           ? membufop(ptr, MEMHASHCHK, NULL, 0)
                           : 0);
    struct membuf *buf = (struct membuf *)desc;
#endif
    size_t         sz = 0;
    MEMUWORD_T     type;
    MEMUWORD_T     slot;
    
    if (buf) {
        type = memgetbuftype(buf);
        slot = memgetbufslot(buf);
        sz = membufblksize(buf, type, slot);
    }

    return sz;
}

size_t
malloc_good_size(size_t size)
{
    size_t sz = 0;
    
#if (WORDSIZE == 4)
    if (memusesmallbuf(sz)) {
        ceilpow2_32(size, sz);
    } else if (memusepagebuf(sz)) {
        sz = rounduppow2(sz, PAGESIZE);
    } else {
        ceilpow2_32(size, sz);
    }
#elif (WORDSIZE == 8)
    if (memusesmallbuf(sz)) {
        ceilpow2_64(size, sz);
    } else if (memusepagebuf(sz)) {
        sz = rounduppow2(sz, PAGESIZE);
    } else {
        ceilpow2_64(size, sz);
    }
#endif

    return sz;
}

size_t
malloc_size(void *ptr)
{
#if (MEMMULTITAB)
    struct membuf *buf = (ptr) ? memfindbuf(ptr, 0) : NULL;
#else
    MEMADR_T       desc = ((ptr)
                           ? membufop(ptr, MEMHASHCHK, NULL, 0)
                           : 0);
    struct membuf *buf = (struct membuf *)desc;
#endif
    size_t         sz = 0;
    MEMUWORD_T     type;
    MEMUWORD_T     slot;

    if (buf) {
        type = memgetbuftype(buf);
        slot = memgetbufslot(buf);
        sz = membufblksize(buf, type, slot);
    }

    return sz;
}

