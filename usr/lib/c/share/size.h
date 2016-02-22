#ifndef __SHARE_SIZE_H__
#define __SHARE_SIZE_H__

//#include <zero/param.h>

#if !defined(__size_t_defined)
#if defined(_MSC_VER)
typedef unsigned long long size_t;
#else
typedef unsigned long      size_t;
#endif
#define __size_t_defined 1
#endif

#endif /* __SHARE_SIZE_H__ */

