#ifndef __ZERO_GNU_H__
#define __ZERO_GNU_H__

#if defined(GNUTRACE) && (GNUTRACE)

#include <stddef.h>
#include <execinfo.h>
#include <zero/cdecl.h>

#define GNUNTRACEFUNC 32

#define trace()      trace3(__func__, __FILE__, __LINE__)
#define trace_fd(fd) trace_fd4(__func__, __FILE__, __LINE__, (fd))

/* print backtrace of maximum GNUTRACEFUNC nested function calls */
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

/* like trace3(), but no allocations done so good for debugging zero malloc :) */
static __inline__ void
trace_fd4(const char *func, char *file, int line, int fd)
{
    void   *buf[GNUNTRACEFUNC];
    size_t  n;

    n = backtrace(buf, GNUNTRACEFUNC);
    backtrace_symbols_fd(buf, n, fd);

    return;
}

#endif /* GNUTRACE */

#endif /* __ZERO_GNU_H__ */

