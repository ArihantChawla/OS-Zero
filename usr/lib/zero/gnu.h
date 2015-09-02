#ifndef __ZERO_GNU_H__
#define __ZERO_GNU_H__

#include <stddef.h>
#include <execinfo.h>
#include <zero/cdecl.h>

#define GNUNTRACEFUNC 32

#define trace()    trace3(__func__, __FILE__, __LINE__)
#define trace_fd() trace_fd3(__func__, __FILE__, __LINE__)

static __inline__ void
trace3(const char *func, char *file, int line)
{
    void    *buf[GNUNTRACEFUNC];
    size_t   n;
    char   **names;
    size_t   ndx;

    n = backtrace(buf, GNUNTRACEFUNC);
    names = backtrace_symbols(buf, n);
    for (ndx = 0 ; ndx < n ; ndx++) {
        fprintf(stderr, "%p : %s\n", buf[ndx], names[ndx]);
    }
    free(names);

    return;
}

static __inline__ void
trace_fd3(const char *func, char *file, int line)
{
    void    *buf[GNUNTRACEFUNC];
    size_t   n;

    n = backtrace(buf, GNUNTRACEFUNC);
    backtrace_symbols_fd(buf, n, STDERR_FILENO);

    return;
}

#endif /* __ZERO_GNU_H__ */

