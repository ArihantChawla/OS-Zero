#ifndef __STDDEF_H__
#define __STDDEF_H__

#include <features.h>
#include <stdint.h>
#include <zero/param.h>

#define NULL           ((void *)0L)

typedef long           ptrdiff_t;
typedef unsigned long  size_t;

#if defined(__STDC_ISO_10646__) || defined(_ZERO_SOURCE)
/* full ISO10646 character */
#define WCHARSIZE      4
#else
/* 16-bit Unicode character */
#define WCHARSIZE      2
#endif
#if (WCHARSIZE == 4)
typedef int32_t        wchar_t;
#elif (WCHARSIZE == 2)
typedef uint16_t       wchar_t;
#elif (WCHARSIZE == 1)
typedef uint8_t        wchar_t;
#endif
typedef int            wint_t;

#if (_ISOC11_SOURCE)
typedef long long      max_align_t;
#endif /* _ISOC11_SOURCE */

#define offsetof(t, m) ((size_t)(&((t *)0)->m))

#endif /* __STDDEF_H__ */

