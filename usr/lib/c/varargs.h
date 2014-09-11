#ifndef __VARARGS_H__
#define __VARARGS_H__

/* implementation of old-style Unix <varargs.h> using later <stdarg.h> */

#include <stdarg.h>

#if defined(__GNUC__)
#define va_alist      __builtin_va_alist
#endif

#define va_dcl        int va_alist;
#undef va_start
#define va_start(v)   ((v) = (va_list)&va_alist)
#undef va_end
#define va_end        ((void)0)
#undef va_arg
#define _va_argsiz(l)                                                   \
    (((sizeof(l) + sizeof(int) - 1) / sizeof(int)) * sizeof(int))
#define va_arg(v, l)  ((v) = (v) + _va_argsiz(l),                        \
                       *((l *)(void *)((v) - _va_argsiz(l))))

#endif /* __VARARGS_H__ */

