#include <limits.h>
#include <unistd.h>
#include <zero/cdecl.h>
#include <zero/param.h>
#include <kern/sys.h>
#include <kern/conf.h>

static struct sys sys ALIGNED(PAGESIZE);

void
sysinit(void)
{
    sys.conf.vals[_SC_OS_VERSION] = _ZERO_VERSION;
    sys.conf.vals[_SC_VERSION] = _POSIX_VERSION;
    sys.conf.vals[_SC_ARG_MAX] = ARG_MAX;
    sys.conf.vals[_SC_CHILD_MAX] = CHILD_MAX;
    sys.conf.vals[_SC_HOST_NAME_MAX] = HOST_NAME_MAX;
    sys.conf.vals[_SC_LOGIN_NAME_MAX] = LOGIN_NAME_MAX;
    sys.conf.vals[_SC_CLK_TCK] = HZ;
    sys.conf.vals[_SC_OPEN_MAX] = OPEN_MAX;
    sys.conf.vals[_SC_PAGESIZE] = PAGESIZE;
    sys.conf.vals[_SC_RE_DUP_MAX] = RE_DUP_MAX;
    sys.conf.vals[_SC_STREAM_MAX] = STREAM_MAX;
    sys.conf.vals[_SC_SYMLOOP_MAX] = SYMLOOP_MAX;
    sys.conf.vals[_SC_TTY_NAME_MAX] = TTY_NAME_MAX;
    sys.conf.vals[_SC_TZNAME_MAX] = TZNAME_MAX;
/* POSIX.2 values - FIXME: set to something meaningful */
    sys.conf.vals[_SC_BC_BASE_MAX] = 0;
    sys.conf.vals[_SC_BC_DIM_MAX] = 0;
    sys.conf.vals[_SC_BC_SCALE_MAX] = 0;
    sys.conf.vals[_SC_BC_STRING_MAX] = 0;
    sys.conf.vals[_SC_COLL_WEIGHTS_MAX] = 0;
    sys.conf.vals[_SC_EXPR_NEST_MAX] = 0;
    sys.conf.vals[_SC_LINE_MAX] = 0;
    sys.conf.vals[_SC_RE_DUP_MAX] = 0;
    sys.conf.vals[_SC_2_VERSION] = 0;
    sys.conf.vals[_SC_2_C_DEV] = 0;
    sys.conf.vals[_SC2_FORT_DEV] = 0;
    sys.conf.vals[_SC_2_LOCALEDEF] = 0;
    sys.conf.vals[_SC2_SW_DEV] = 0;
/* possibly non-standard values - FIXME: phys pages, nprocs */
    sys.conf.vals[_SC_PHYS_PAGES] = 0;
    sys.conf.vals[_SC_AVPHYS_PAGES] = 0;
    sys.conf.vals[_SC_NPROCESSORS_CONF] = 1;
    sys.conf.vals[_SC_NPROCESSORS_ONLN] = 1;
//    sys.conf.vals[_SC_CACHELINE_SIZE] = CLSIZE;
    sys.clsize = CLSIZE;
}

long
sysgetconf(int scval)
{
    long retval;
    
    if (scval < 0 || scval >= NSYSCONF) {
//        kseterrno(EINVAL);
        
        return -1;
    }
    retval = sys.conf.vals[scval];
    
    return retval; 
}

long
sysgetclsize(void)
{
    long retval = sys.clsize;

    return retval;
}

