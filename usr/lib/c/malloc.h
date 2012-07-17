#ifndef __MALLOC_H__
#define __MALLOC_H__

#include <features.h>

void * memalign(size_t align, size_t size);
#if (_GNU_SOURCE)
void * pvalloc(size_t size);
#endif

#endif /* __MALLOC_H__ */

