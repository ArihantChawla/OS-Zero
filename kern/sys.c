#include <kern/conf.h>
#include <limits.h>
#include <unistd.h>
#include <mach/param.h>
#include <kern/version.h>
#include <kern/cpu.h>
#include <kern/sys.h>
#include <kern/io/buf.h>

static struct sys k_sys ALIGNED(PAGESIZE);

void
sysinit(void)
{
    volatile struct m_cpucacheinfo  cbuf;
    long                           *tab = k_sys.conf.tab;
    long                           *ptr = tab + zeroabs(MINSYSCONF);

    ptr[_SC_OS_VERSION] = _ZERO_VERSION;
    ptr[_SC_VERSION] = _POSIX_VERSION;
    ptr[_SC_ARG_MAX] = ARG_MAX;
    ptr[_SC_CHILD_MAX] = CHILD_MAX;
    ptr[_SC_HOST_NAME_MAX] = HOST_NAME_MAX;
    ptr[_SC_LOGIN_NAME_MAX] = LOGIN_NAME_MAX;
    ptr[_SC_CLK_TCK] = HZ;
    ptr[_SC_OPEN_MAX] = OPEN_MAX;
    ptr[_SC_PAGE_SIZE] = PAGESIZE;
    ptr[_SC_RE_DUP_MAX] = RE_DUP_MAX;
    ptr[_SC_STREAM_MAX] = STREAM_MAX;
    ptr[_SC_SYMLOOP_MAX] = SYMLOOP_MAX;
    ptr[_SC_TTY_NAME_MAX] = TTY_NAME_MAX;
    ptr[_SC_TZNAME_MAX] = TZNAME_MAX;
/* POSIX.2 values - FIXME: set to something meaningful */
    ptr[_SC_BC_BASE_MAX] = 0;
    ptr[_SC_BC_DIM_MAX] = 0;
    ptr[_SC_BC_SCALE_MAX] = 0;
    ptr[_SC_BC_STRING_MAX] = 0;
    ptr[_SC_COLL_WEIGHTS_MAX] = 0;
    ptr[_SC_EXPR_NEST_MAX] = 0;
    ptr[_SC_LINE_MAX] = 0;
    ptr[_SC_RE_DUP_MAX] = 0;
    ptr[_SC_2_VERSION] = 0;
    ptr[_SC_2_C_DEV] = 0;
    ptr[_SC2_FORT_DEV] = 0;
    ptr[_SC_2_LOCALEDEF] = 0;
    ptr[_SC2_SW_DEV] = 0;
/* possibly non-standard values - FIXME: phys pages, nprocs */
    ptr[_SC_PHYS_PAGES] = 0;
    ptr[_SC_AVPHYS_PAGES] = 0;
    ptr[_SC_NPROCESSORS_CONF] = 1;
    ptr[_SC_NPROCESSORS_ONLN] = 1;
#if 0
    ptr[_SC_CACHELINE_SIZE] = cpugetclsize(&cbuf);
    ptr[_SC_NTLB] = cpugetntlb(&cbuf);
    ptr[_SC_L1_INST_SIZE] = cpugetl1isize(&cbuf);
    ptr[_SC_L1_DATA_SIZE] = cpugetl1dsize(&cbuf);;
    ptr[_SC_L1_INST_NWAY] = cpugetl1inway(&cbuf);;
    ptr[_SC_L1_DATA_NWAY] = cpugetl1dnway(&cbuf);;
    ptr[_SC_L2_SIZE] = cpugetl2size(&cbuf);;
    ptr[_SC_L2_NWAY] = cpugetl2nway(&cbuf);;
#endif
    ptr[_SC_BUF_BLK_SIZE] = BUFMAXSIZE;
    k_sys.conf.ptr = ptr;

    return;
}

long
sysgetconf(int scval)
{
    long *ptr = k_sys.conf.tab;
    long  retval;

    if (scval < MINSYSCONF || scval > MAXSYSCONF) {
//        kseterrno(EINVAL);

        return -1;
    }
    retval = ptr[scval];

    return retval;
}

