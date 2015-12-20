#ifndef __EXECINFO_H__
#define __EXECINFO_H__

#include <features.h>

#if defined(_GNU_SOURCE)
extern uintptr_t _backtrace(void **buf, int size, long syms, int fd);
extern uintptr_t _btprintf(void **buf, int size, const char *fmt, int fd);
#define          backtrace(buf, size)                                   \
    (int)_backtrace(buf, size, 0, -1)
#define          backtrace_symbols(buf, size)                           \
    (char **)_backtrace(NULL, size, 1, -1)
#define          backtrace_symbols_fd(buf, size, fd)                    \
    (void)_backtrace(NULL, size, 1, fd)
#define          backtrace_symbols_fmt(buf, size, fmt)                  \
    (char **)_btprintf(buf, size, fmt, -1)
#define          backtrace_symbols_fmt_fd(buf, size, fmt, fd)           \
    (char **)_btprintf(buf, size, fmt, fd)
#endif /* defined(_GNU_SOURCE) */

#endif /* __EXECINFO_H__ */

