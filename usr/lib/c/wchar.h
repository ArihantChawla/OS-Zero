#ifndef __WCHAR_H__
#define __WCHAR_H__

#include <features.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

struct               tm;

#define WCHAR_MIN    0
#if (WCHARSIZE == 4)
#define WEOF         ((wint_t)0xffffffff) /* -1 */
#define WCHAR_MAX    0x0010ffff
#define WCHARNBIT    21
#elif (WCHARSIZE == 2)
#define WEOF         ((wint_t)0xffff)	/* 0xffff is invalid character value in Unicode */
#define WCHAR_MAX    0xffffU
#define WCHARNBIT    16
#elif (WCHARSIZE == 1)
#define WEOF         0x00
#define WCHAR_MAX    0xff
#define WCHARNBIT    8
#endif /* WCHARSIZE */
#define WCHARMASK    ((1 << WCHARNBIT) - 1)

#if (_GNU_SOURCE) || (_XOPEN_SOURCE >= 700 || _POSIX_C_SOURCE >= 200809L)
FILE *open_wmemstream(wchar_t **ptr, size_t *sizeptr);
#endif

#endif /* __WCHAR_H__ */

