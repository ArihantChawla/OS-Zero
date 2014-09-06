#ifndef __WCHAR_H__
#define __WCHAR_H__

#include <stddef.h>

#define WEOF         ((wint_t)0xffff)	/* 0xffff is invalid character value in Unicode */
#define WCHAR_MIN    0
#if (WCHARSIZE == 4)
#define WCHAR_MAX    0x0010ffff
#define WCHARNBIT    21
#elif (WCHARSIZE == 2)
#define WCHAR_MAX    0xffff
#define WCHARNBIT    16
#elif (WCHARSIZE == 1)
#define WCHAR_MAX    0xff
#define WCHARNBIT    8
#endif /* WCHARSIZE */
#define WCHARMASK    ((1 << WCHARNBIT) - 1)

#endif /* __WCHAR_H__ */

