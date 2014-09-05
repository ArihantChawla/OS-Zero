#ifndef __WCHAR_H__
#define __WCHAR_H__

#include <stddef.h>

#define WCHAR_MIN   0
#define WCHAR_MAX   0xffffffff
#define WEOF        ((wint_t)0xffff)	/* 0xffff is invalid character value in Unicode */
#define WCUCMAX     0x0010ffff
#define WCUCMASK    0x001fffff
#define WCNUCBIT    21
#if 0
#define WCUCFLGMASK 0xffe00000
#define WCFLGMASK   0x7ff
#define WCNFLGBIT   11
#endif /* 0 */
/* use high 8 bits for character flag-bits; low 24 bits for character value */
#define WCUCFLGMASK 0xff000000
#define WCFLGMASK   0x000000ff
#define WCNFLGBIT   8
#define WCFLGSHIFT  (32 - WCNFLGBIT)

#endif /* __WCHAR_H__ */

