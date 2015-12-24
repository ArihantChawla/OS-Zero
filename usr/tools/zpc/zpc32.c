#include <zpc/conf.h>
#include <zero/cdefs.h>
#include <zero/param.h>

uintptr_t zpcpagetab[1U << ZPCNPFRMBIT] ALIGNED(PAGESIZE);

