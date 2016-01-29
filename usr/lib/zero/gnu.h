#ifndef __ZERO_GNU_H__
#define __ZERO_GNU_H__

#if defined(GNUTRACE) && (GNUTRACE)

#include <stddef.h>
#include <execinfo.h>
#include <zero/cdefs.h>

#define GNUTRACEMAX 32  // max # of function calls to backtrace

#define trace()      trace3(__func__, __FILE__, __LINE__)
#define trace_fd(fd) trace_fd4(__func__, __FILE__, __LINE__, (fd))

/* print backtrace of maximum GNUTRACEFUNC nested function calls */
static __inline__ void
trace3(const char *func, char *file, int line)
{
    void    *buf[GNUTRACEMAX];
    size_t   n;
    char   **names;
    size_t   ndx;

    fprintf(stderr, "TRACE invoked: %s() @ %s : %d\n", func, file, line);
    n = backtrace(buf, GNUTRACEMAX);
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
    void   *buf[GNUTRACEMAX];
    size_t  n;

    fprintf(stderr, "TRACE invoked: %s() @ %s : %d\n", func, file, line);
    n = backtrace(buf, GNUTRACEMAX);
    backtrace_symbols_fd(buf, n, fd);

    return;
}

#endif /* GNUTRACE */

#endif /* __ZERO_GNU_H__ */

