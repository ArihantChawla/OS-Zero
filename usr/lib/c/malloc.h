#ifndef __MALLOC_H__
#define __MALLOC_H__

#include <stddef.h>

void * malloc(size_t size);
void * calloc(size_t n, size_t size);
void * realloc(void *ptr, size_t size);
void   free(void *ptr);
void * valloc(size_t size);
void * memalign(size_t align, size_t size);
void * pvalloc(size_t size);
void   cfree(void *ptr);


/* TODO: implement SVID2/XPG mallinfo */

struct mallinfo {
	int arena;		// non-mmapped space allocated from system
	int ordblks;	// number of free chunks
	int smblks;		// number of fastbin blocks
	int hblks;		// number of mmapped regions
	int hblkhd;		// space in mmapped register
	int usmblks;	// maximum total allocated space
	int fsmblks;	// space available in freed fastbin blocks
	int uordblks;	// total allocated space
	int fordblks;	// total free space
	int keepcost;	// top-most, releasable space (malloc_trim())
};

extern struct mallinfo mallinfo;

/* mallopt() options */
#define M_MXFAST           1
#define M_NBLKS            2
#define M_GRAIN            3
#define M_KEEP             4
#define M_TRIM_THRESHOLD  -1
#define M_TOP_PAD         -2
#define M_MMAP_THRESHOLD  -3
#define M_MMAP_MAX        -4
#define M_CHECK_ACTION    -5
#define M_PERTURB         -6
#define M_ARENA_TEST      -7
#define M_ARENA_MAX       -8

extern int     mallopt(int parm, int val);
extern int     malloc_trim(size_t pad);
extern size_t  malloc_usable_size(void *ptr);
extern void    malloc_stats(void);
extern void   *malloc_get_state(void);
extern int     malloc_set_state(void *ptr);

#if defined(_GNU_SOURCE) && (GNUMALLOCHOOKS)
extern void *(*__malloc_hook)(size_t size, const void *caller);
extern void *(*__realloc_hook)(void *ptr, size_t size, const void *caller);
extern void *(*__memalign_hook)(size_t align, size_t size, const void *caller);
extern void  (*__free_hook)(void *ptr, const void *caller);
extern void *(*__malloc_initialize_hook)(void);
extern void  (*__after_morecore_hook)(void);
#endif

#endif /* __MALLOC_H__ */

