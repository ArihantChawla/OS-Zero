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
#include "_malloc.h"

extern THREADLOCAL volatile struct memtls *g_memtls;
extern THREADLOCAL zerofmtx                g_memtlsinitlk;
extern THREADLOCAL MEMUWORD_T              g_memtlsinit;
extern struct mem                          g_mem;

static void *
_malloc(size_t size, size_t align, long flg)
{
    size_t    aln = max(align, MEMMINALIGN);
    size_t    sz = max(size, MEMMINBLK);
    size_t    bsz = (aln <= sz) ? sz : sz + aln;
    long      type = (memusesmallbuf(bsz)
                      ? MEMSMALLBUF
                      : (memusepagebuf(bsz)
                         ? MEMPAGEBUF
                         : MEMBIGBUF));
    long      slot;
    void     *ptr;

#if 0
    if (!g_memtls && !meminittls()) {

        abort();
    }
#endif
    if (!g_memtlsinit) {
        fmtxlk(&g_memtlsinitlk);
        if (!g_memtlsinit) {
            meminittls();
        }
        fmtxunlk(&g_memtlsinitlk);
    }
    if (type != MEMPAGEBUF) {
        memcalcslot(bsz, slot);
    } else {
        memcalcpageslot(bsz, slot);
    }
    ptr = memgetblk(slot, type, size, aln);
    if (!ptr) {
#if defined(ENOMEM)
        errno = ENOMEM;
#endif
    } else if (flg & MALLOCZEROBIT) {
        memset(ptr, 0, size);
    }
#if 0
    if (ptr) {
        VALGRINDALLOC(ptr, size, 0, flg & MALLOCZEROBIT);
    }
#endif
#if (MEMDEBUG)
    crash(ptr != NULL);
#endif

    return ptr;
}

static void
_free(void *ptr)
{
#if (MEMMULTITAB)
    struct membuf *buf;
#endif
    
    if (!ptr) {

        return;
    }
#if (MEMTLSINITFIXED)
    if (!g_memtlsinit) {
        fmtxtrylk(&g_memtlsinitlk);
        if (!g_memtlsinit) {

            abort();
        }
        fmtxunlk(&g_memtlsinitlk);
    }
#else
    if (!g_memtlsinit) {
        fmtxlk(&g_memtlsinitlk);
        if (!g_memtlsinit) {
            meminittls();
        }
        fmtxunlk(&g_memtlsinitlk);
    }
#endif
#if (MEMMULTITAB)
    memfindbuf(ptr, 1);
#else
    membufop(ptr, MEMHASHDEL, NULL);
#endif
#if 0
    VALGRINDFREE(ptr);
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
    MEMADR_T       desc = ((ptr)
                           ? membufop(ptr, MEMHASHCHK, NULL)
                           : 0);
    struct membuf *buf = (struct membuf *)desc;
#endif
//    MEMPTR_T       oldptr = (buf) ? membufgetptr(buf, ptr) : NULL;
    MEMUWORD_T     type;
    MEMUWORD_T     slot;
    size_t         sz;

    if (ptr) {
        type = memgetbuftype(buf);
        slot = memgetbufslot(buf);
        sz = membufblksize(buf, type, slot);
        if (size < sz) {

            return ptr;
        }
    }
    retptr = _malloc(size, 0, 0);
    if (retptr) {
        if (ptr) {
            memcpy(retptr, ptr, sz);
            /* TODO: we looked the buf up already */
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
    void   *ptr = NULL;

    if (!powerof2(align) || (align & (sizeof(void *) - 1))) {
        errno = EINVAL;
        *ret = NULL;

        return -1;
    } else {
        ptr = _malloc(size, align, 0);
        if (!ptr) {
            *ret = NULL;
            
            return -1;
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
    _free(ptr);

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
    void   *ptr = _malloc(rounduppow2(size, PAGESIZE), PAGESIZE, 0);

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
                           ? membufop(ptr, MEMHASHCHK, NULL)
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
                           ? membufop(ptr, MEMHASHCHK, NULL)
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

