#include <unistd.h>
#include <zero/cdecl.h>
#include <zero/param.h>

int
getpagesize(void)
{
#if defined(PAGESIZE)
    int retval = PAGESIZE;
#elif defined(_SC_PAGESIZE)
    int retval = sysconf(_SC_PAGESIZE);
#elif defined(_SC_PAGE_SIZE)
    int retval = sysconf(_SC_PAGE_SIZE);
#endif

    return retval;
}

