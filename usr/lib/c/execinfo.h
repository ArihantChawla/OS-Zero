#ifndef __EXECINFO_H__
#define __EXECINFO_H__

#include <features.h>

#if defined(_GNU_SOURCE)
extern uintptr_t _backtrace(void **buf, int size, long syms, int fd);
#define backtrace(buf, sz)                (int)_backtrace(buf, sz, 0, -1)
#define backtrace_symbols(buf, sz)        (char **)_backtrace(NULL, sz, 1, -1)
#define backtrace_symbols_fd(buf, sz, fd) (void)_backtrace(NULL, sz, 1, fd)
#endif /* defined(_GNU_SOURCE) */

#endif /* __EXECINFO_H__ */

