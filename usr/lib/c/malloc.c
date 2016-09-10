#include <stdint.h>
#include <errno.h>
#include <zero/cdefs.h>
#include <zero/param.h>
#include <zero/mem.h>
#include "_malloc.h"

static struct malloc g_malloc ALIGNED(PAGESIZE);

static void *
_malloc(size_t size, size_t align, long flg)
{
    size_t  sz = (align <= size) ? size : size + align;
    long    type = (memusesmallbuf(sz)
                    ? MEMSMALLBLK
                    : (memusepagebuf(sz)
                       ? MEMPAGEBLK
                       : MEMBIGBLK));
    long    slot = memcalcbufslot(sz, type);
    void   *ptr = memgetblk(slot, type);

    if (!ptr) {
#if defined(ENOMEM)
        errno = ENOMEM;
#endif
    }

    return ptr;
}

static void
_free(void *ptr)
{
    ;
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

void
free(void *ptr)
{
    _free(ptr);

    return;
}

