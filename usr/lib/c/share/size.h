#ifndef __SHARE_SIZE_H__
#define __SHARE_SIZE_H__

#include <zero/param.h>

#if !defined(__size_t_defined)
#if (LONGSIZE == WORDSIZE)
typedef unsigned long      size_t;
#elif (LONGLONGSIZE == WORDSIZE)
typedef unsigned long long size_t;
#endif
#define __size_t_defined 1
#endif

#endif /* __SHARE_SIZE_H__ */

