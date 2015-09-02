#define USE_GETRLIMIT 0
#define USE_SYSCONF   1

#if (USE_GETRLIMIT)
#include <sys/resource.h>
#else
#include <unistd.h>
#endif

#if (USE_SYSCONF) && defined(_SC_OPEN_MAX)

/* query maximum number of open files using sysconf() */
int
get_open_max(void)
{
    int retval;

    retval = sysconf(_SC_OPEN_MAX);

    return retval;
}

#elif (USE_GETRLIMIT) && defined(RLIMIT_NOFILE)

/* query maximum number of open files using getrlimit() */
int
get_open_max(void)
{
    int           retval = -1;
    struct rlimit rlimit;

    if (getrlimit(RLIMIT_NOFILE, &rlimit) == 0
        && rlimit.rlimit_cur != RLIM_INFINITY) {
        retval = rlimit.rlim_cur;
    }

    return retval;
}

#else

/* query maximum number of open files using sysdtablesize() */
int
get_open_max(void)
{
    int retval;

    retval = getdtablesize();

    return retval;
}

#endif

