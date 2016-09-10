#include <stdint.h>
#include <zero/cdefs.h>
#include <zero/param.h>
#include <zero/mem.h>
#include "_malloc.h"

static struct malloc g_malloc ALIGNED(PAGESIZE);

static void *
_malloc(size_t size, size_t align, long flg)
{
    size_t sz = (align <= size) ? size : size + align;
}

static void
_free(void *ptr)
{
}

