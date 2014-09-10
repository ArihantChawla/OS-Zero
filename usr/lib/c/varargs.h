#ifndef __VARARGS_H__
#define __VARARGS_H__

#if !defined(__STDARG_H__)

https://gcc.gnu.org/ml/gcc-patches/2002-07/msg00737.html

#if defined(__GNUC__)
typedef __builtin_va_alist va_alist;
#endif

#endif /* __VARARGS_H__ */

