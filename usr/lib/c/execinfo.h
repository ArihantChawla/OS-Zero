#ifndef __EXECINFO_H__
#define __EXECINFO_H__

#include <features.h>

#if defined(_BSD_SOURCE)
#include <stddef.h>
typedef size_t __btsize_t;
#endif

#if defined(_GNU_SOURCE) ||defined(_BSD_SOURCE)
extern uintptr_t _backtrace(void **buf, __btsize_t size, long syms, int fd);
#if defined(_BSD_SOURCE)
extern uintptr_t _btprintf(void **buf, __btsize_t size, const char *fmt,
                           int fd);
#endif
#define          backtrace(buf, size)                                   \
    (int)_backtrace(buf, size, 0, -1)
#define          backtrace_symbols(buf, size)                           \
    (char **)_backtrace(NULL, size, 1, -1)
#define          backtrace_symbols_fd(buf, size, fd)                    \
    (void)_backtrace(NULL, size, 1, fd)
#if defined(_BSD_SOURCE)
#define          backtrace_symbols_fmt(buf, size, fmt)                  \
    (char **)_btprintf(buf, size, fmt, -1)
#define          backtrace_symbols_fmt_fd(buf, size, fmt, fd)           \
    (char **)_btprintf(buf, size, fmt, fd)
#endif
#endif /* defined(_GNU_SOURCE) || defined(_BSD_SOURCE) */

#endif /* __EXECINFO_H__ */

