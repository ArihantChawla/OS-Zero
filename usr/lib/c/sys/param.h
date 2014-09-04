#ifndef __SYS_PARAM_H__
#define __SYS_PARAM_H__

#include <limits.h>
#include <zero/param.h>
#include <zero/trix.h>

/* BSD names for some <limits.h> values */
#define NBBY          CHAR_BIT
#if !defined(NGROUPS)
#define NGROUPS       NGROUPS_MAX
#endif
#define MAXSYMLINKS   16
#define CANBSIZE     MAX_CANON
#define NCARGS        ARG_MAX
#define MAXPATHLEN    PATH_MAX
#define NOFILE        OPEN_MAX

#ifndef howmany
#define howmany(x, y) (((x) + ((y) - 1)) / (y))
#endif
#define roundup(x, y) ((((x) + ((y) - 1)) / (y)) * (y))
#define MIN(a, b)     min((a), (b))
#define MAX(a, b)     max((a), (b))

/* unit for st_blocks */
#define DEV_BSIZE     512

#endif /* __SYS_PARAM_H__ */

