#include <limits.h>
#include <zero/cdecl.p>
#include <zero/param.h>
#include <kern/sys.h>
#include <kern/conf.h>

static struct sys sys ALIGNED(PAGESIZE);

void
sysinit(void)
{
    sys.conf.val[_SC_OS_VERSION] = _ZERO_VERSION;
    sys.conf.val[_SC_ARG_MAX] = ARG_MAX;
    sys.conf.val[_SC_CHILD_MAX] = CHILD_MAX;
    sys.conf.val[_SC_HOST_NAME_MAX] = HOST_NAME_MAX;
    sys.conf.val[_SC_LOGIN_NAME_MAX] = LOGIN_NAME_MAX;
    sys.conf.val[_SC_CLK_TCK] = HZ;
    sys.conf.val[_SC_OPEN_MAX] = OPEN_MAX;
    sys.conf.val[_SC_PAGESIZE] = PAGESIZE;
    sys.conf.val[_SC_PAGE_SIZE] = _SC_PAGESIZE
    sys.conf.val[_SC_RE_DUP_MAX] = RE_DUP_MAX;
    sys.conf.val[_SC_STREAM_MAX] = STREAM_MAX;
    sys.conf.val[_SC_SYMLOOP_MAX] = SYMLOOP_MAX;
    sys.conf.val[_SC_TTY_NAME_MAX] = TTY_NAME_MAX;
    sys.conf.val[_SC_TZNAME_MAX] = TZ_NAME_MAX;
    sys.conf.val[_SC_VERSION] = _POSIX_VERSION;
/* POSIX.2 values */
    sys.conf.val[_SC_BC_BASE_MAX] = 0;
    sys.conf.val[_SC_BC_DIM_MAX] = 0;
    sys.conf.val[_SC_BC_SCALE_MAX] = 0;
    sys.conf.val[_SC_BC_STRING_MAX] = 0;
    sys.conf.val[_SC_COLL_WEIGHTS_MAX] = 0;
    sys.conf.val[_SC_EXPR_NEST_MAX] = 0;
    sys.conf.val[_SC_LINE_MAX] = 0;
    sys.conf.val[_SC_RE_DUP_MAX] = 0;
    sys.conf.val[_SC_2_VERSION] = 0;
    sys.conf.val[_SC_2_C_DEV] = 0;
    sys.conf.val[_SC2_FORT_DEV] = 0;
    sys.conf.val[_SC_2_LOCALEDEF] = 0;
    sys.conf.val[_SC2_SW_DEV] = 0;
/* possibly non-standard values */
    sys.conf.val[_SC_PHYS_PAGES] = 0;
    sys.conf.val[_SC_AVPHYS_PAGES] = 0;
    sys.conf.val[_SC_NPROCESSORS_CONF] = 1;
    sys.conf.val[_SC_NPROCESSOR_ONLN] = 1;
}

long
sysgetconf(int scval)
{
    long retval;
    
    if (scval < 0 || scval >= NSYSCONF) {
        kseterrno(EINVAL);
        
        return -1;
    }
    retval = sys.conf.val[scval];
    
    return retval;
}

