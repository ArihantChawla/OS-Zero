#ifndef __SHARE_WCHAR_H__
#define __SHARE_WCHAR_H__

#if !defined(__wchar_t_defined)

#include <zero/param.h>

#if defined(__STDC_ISO_10646__) || defined(_ZERO_SOURCE)
/* full ISO10646 character */
#define WCHARSIZE      4
#elif (defined(_WIN32) || defined(_WIN64))
/* 16-bit Unicode character */
#define WCHARSIZE      2
#else
#error WCHARSIZE not present in <stddef.h>
#endif

#if (WCHARSIZE == 4)
typedef int32_t  wchar_t;
#elif (WCHARSIZE == 2)
typedef uint16_t wchar_t;
#elif (WCHARSIZE == 1)
typedef uint8_t  wchar_t;
#endif

#define __wchar_t_defined 1

#endif /* !defined(__wchar_t_defined) */

#endif /* __SHARE_WCHAR_H__ */
