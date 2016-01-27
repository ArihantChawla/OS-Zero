#ifndef __SYS_PARAM_H__
#define __SYS_PARAM_H__

//#include <zero/trix.h>

/* BSD names for some <limits.h> values */
#define NBBY          8
#if !defined(NGROUPS)
#define NGROUPS       NGROUPS_MAX
#endif
#define MAXSYMLINKS   16
#define CANBSIZE      MAX_CANON
#define NCARGS        ARG_MAX
#define MAXPATHLEN    PATH_MAX
#define NOFILE        OPEN_MAX

#ifndef howmany
#define howmany(a, b) (((a) + ((b) - 1)) / (b))
#endif
#if defined(__GNUC__) && 0
#define roundup(a, b)                                                   \
    ((__builtin_constant_p(b) && powerof2(b))                             \
     ? rounduppow2(a, b)                                                \
     : ((((a) + ((b) - 1)) / (b)) * b))
#else
#define roundup(a, b)                                                   \
    ((((a) + ((b) - 1)) / (b)) * (b))
#endif
#define MIN(a, b)     min((a), (b))
#define MAX(a, b)     max((a), (b))

/* unit for st_blocks */
#define DEV_BSIZE     512

#endif /* __SYS_PARAM_H__ */

