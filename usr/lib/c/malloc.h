#ifndef __MALLOC_H__
#define __MALLOC_H__

#include <features.h>

void * malloc(size_t size);
void * calloc(size_t n, size_t size);
void * realloc(void *ptr, size_t size);
void   free(void *ptr);
void * valloc(size_t size);
void * memalign(size_t align, size_t size);
void * pvalloc(size_t size);
void   cfree(void *ptr);

#endif /* __MALLOC_H__ */

