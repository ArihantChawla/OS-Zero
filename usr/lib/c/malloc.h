#ifndef __MALLOC_H__
#define __MALLOC_H__

#if !defined(__KERNEL__)

#include <features.h>
#include <stddef.h>
#if (_GNU_SOURCE)
#include <stdio.h>
#endif

extern void * memalign(size_t align, size_t size);
extern void * pvalloc(size_t size);
extern void   cfree(void *ptr);

extern int    malloc_info(int opt, FILE *fp);

extern void * _aligned_malloc(size_t size, size_t align);
extern void   _aligned_free(void *);
#define
#endif
#if defined(_INTEL_SOURCE) && !defined(__GNUC__)
extern void * _mm_malloc(size_t size, size_t align);
extern void   _mm_free(void *ptr);
#endif
#if defined(_MSVC_SOURCE)
#define coreleft() _memavl()
#endif

/* TODO: implement SVID2/XPG mallinfo; might need long-fields... */

struct mallinfo {
    int arena;		// non-mmapped space allocated from system
    int ordblks;	// number of free chunks
    int smblks;		// number of fastbin blocks
    int hblks;		// number of mmapped regions
    int hblkhd;		// space in mmapped register
    int usmblks;	// maximum total allocated space
    int fsmblks;	// space available in freed fastbin blocks
    int uordblks;	// total allocated space
    int fordblks;       // total free space
    int keepcost;       // top-most, releasable space (malloc_trim())
};

extern struct mallinfo mallinfo(void);

/* mallopt() options */
#define M_MXFAST           1
#define M_NLBLKS           2
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

extern int      mallopt(int parm, int val);
extern int      malloc_trim(size_t pad);
extern size_t   malloc_usable_size(void *ptr);
extern void     malloc_stats(void);
extern void   * malloc_get_state(void);
extern int      malloc_set_state(void *ptr);

#endif /* !defined(__KERNEL__) */

#endif /* __MALLOC_H__ */

