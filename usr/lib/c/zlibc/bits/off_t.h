#ifndef __BITS_OFF_T_H__
#define __BITS_OFF_T_H__

#include <features.h>
#if !defined(__off_t_defined)
#include <stdint.h>
#if (_FILE_OFFSET_BITS == 64)
typedef int64_t         off_t;
#else
typedef int32_t         off_t;
#endif
#if (_FILE_OFFSET_BITS == 64)
typedef int64_t         off_t;
#else
typedef int32_t         off_t;
#endif
#if defined(_LARGEFILE64_SOURCE)
typedef int64_t         off64_t;
#endif
#define __off_t_defined 1
#endif /* !defined(__off_t_defined) */

#endif /* __BITS_OFF_T_H__ */

