#ifndef __ALLOC_H__
#define __ALLOC_H__

#include <features.h>
#include <alloca.h>
#include <stdlib.h>
#include <share/stddef.h>

#if (defined(_MSDOS_SOURCE)                                             \
     || defined(_BORLANDC_SOURCE) || defined(_TURBOC_SOURCE))

#define  farmalloc(sz)       malloc(sz)
#define  farfree(ptr)        free(ptr)
#define  farcalloc(n, sz)    calloc(n, sz)
#define  farrealloc(ptr, sz) realloc(ptr, sz)
int      brk(void *adr);
void    *sbrk(intptr_t incr);

#endif

#endif /* __ALLOC_H__ */

