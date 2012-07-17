/*
 * math.h - internal mathematics header for Earthquake C Environment.
 * Copyright (C) 2004 Tuomo Venäläinen
 *
 * See the file COPYING for information about using this software.
 */

#ifndef EARTHQUAKE_INTERNAL_MATH_H
#define EARTHQUAKE_INTERNAL_MATH_H

int __Eatoi(const char *str);
long __Eatol(const char *str);
long long __Eatoll(const char *str);
double __Eatof(const char *str);
long double __Eatold(const char *str);
long __Eatow(const char *str);
#if (__ECC_INT_SIZE == 4)
#   define __Eatoi32(s) __Eatoi(s)
#elif (__ECC_LONG_SIZE == 4)
#   define __Eatoi32(s) _ctatol(s)
#endif
#if (__ECC_INT_SIZE == 8)
#   define __Eatoi64(s) __Eatoi(s)
#elif (__ECC_LONG_SIZE == 8)
#   define __Eatoi64(s) _ctatol(s)
#elif (__ECC_LONG_LONG_SIZE == 8)
#   define __Eatoi64(s) _ctatoll(s)
#endif

unsigned int __Eatoui(const char *str);
unsigned long __Eatoul(const char *str);
unsigned long long __Eatoull(const char *str);
unsigned long __Eatouw(const char *str);
#if (__ECC_UNSIGNED_INT_SIZE == 4)
#   define __Eatoui32(s) __Eatoui(s)
#elif (__ECC_UNSIGNED_LONG_SIZE == 4)
#   define __Eatoui32(s) _ctatol(s)
#endif
#if (__ECC_UNSIGNED_INT_SIZE == 8)
#   define __Eatoui64(s) __Eatoi(s)
#elif (__ECC_UNSIGNED_LONG_SIZE == 8)
#   define __Eatoui64(s) _ctatoul(s)
#elif (__ECC_UNSIGNED_LONG_LONG_SIZE == 8)
#   define __Eatoui64(s) _ctatoull(s)
#endif

long __Ehibit(long val);
long __Elobit(long val);
long __Ehibitu(unsigned long val);
long __Elobitu(unsigned long val);
long __Emaxshift(long val);
long __Emaxshiftu(unsigned long val);

#endif /* EARTHQUAKE_INTERNAL_MATH_H */

