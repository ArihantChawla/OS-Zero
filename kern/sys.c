#include <limits.h>
#include <unistd.h>
#include <zero/cdefs.h>
#include <zero/param.h>
#include <kern/conf.h>
#include <kern/sys.h>
#include <kern/io/buf.h>

static struct sys systab[NCPU] ALIGNED(PAGESIZE);

void
sysinit(long id)
{
    long             *tab = systab[id].conf.tab;
    struct m_cpuinfo *cpuinfo = k_cpuinfo;
    
    tab[_SC_OS_VERSION] = _ZERO_VERSION;
    tab[_SC_VERSION] = _POSIX_VERSION;
    tab[_SC_ARG_MAX] = ARG_MAX;
    tab[_SC_CHILD_MAX] = CHILD_MAX;
    tab[_SC_HOST_NAME_MAX] = HOST_NAME_MAX;
    tab[_SC_LOGIN_NAME_MAX] = LOGIN_NAME_MAX;
    tab[_SC_CLK_TCK] = HZ;
    tab[_SC_OPEN_MAX] = OPEN_MAX;
    tab[_SC_PAGE_SIZE] = PAGESIZE;
    tab[_SC_RE_DUP_MAX] = RE_DUP_MAX;
    tab[_SC_STREAM_MAX] = STREAM_MAX;
    tab[_SC_SYMLOOP_MAX] = SYMLOOP_MAX;
    tab[_SC_TTY_NAME_MAX] = TTY_NAME_MAX;
    tab[_SC_TZNAME_MAX] = TZNAME_MAX;
/* POSIX.2 values - FIXME: set to something meaningful */
    tab[_SC_BC_BASE_MAX] = 0;
    tab[_SC_BC_DIM_MAX] = 0;
    tab[_SC_BC_SCALE_MAX] = 0;
    tab[_SC_BC_STRING_MAX] = 0;
    tab[_SC_COLL_WEIGHTS_MAX] = 0;
    tab[_SC_EXPR_NEST_MAX] = 0;
    tab[_SC_LINE_MAX] = 0;
    tab[_SC_RE_DUP_MAX] = 0;
    tab[_SC_2_VERSION] = 0;
    tab[_SC_2_C_DEV] = 0;
    tab[_SC2_FORT_DEV] = 0;
    tab[_SC_2_LOCALEDEF] = 0;
    tab[_SC2_SW_DEV] = 0;
/* possibly non-standard values - FIXME: phys pages, nprocs */
    tab[_SC_PHYS_PAGES] = 0;
    tab[_SC_AVPHYS_PAGES] = 0;
    tab[_SC_NPROCESSORS_CONF] = 1;
    tab[_SC_NPROCESSORS_ONLN] = 1;
//    sys.conf.tab[_SC_CACHELINE_SIZE] = CLSIZE;
    tab[_SC_CACHELINE_SIZE] = cpugetclsize(cpuinfo);
    tab[_SC_NTLB] = cpugetntlb(cpuinfo);
    tab[_SC_L1_INST_SIZE] = cpugetl1isize(cpuinfo);
    tab[_SC_L1_DATA_SIZE] = cpugetl1dsize(cpuinfo);;
    tab[_SC_L1_INST_NWAY] = cpugetl1inway(cpuinfo);;
    tab[_SC_L1_DATA_NWAY] = cpugetl1dnway(cpuinfo);;
    tab[_SC_L2_SIZE] = cpugetl2size(cpuinfo);;
    tab[_SC_L2_NWAY] = cpugetl2nway(cpuinfo);;
    tab[_SC_BUF_BLK_SIZE] = BUFSIZE;
    systab[id].conf.ptr = tab + MINSYSCONF;

    return;
}

long
sysgetconf(int scval)
{
    long id = k_cpuid;
    long retval;
    
    if (scval < MINSYSCONF || scval > MAXSYSCONF) {
//        kseterrno(EINVAL);
        
        return -1;
    }
    retval = systab[id].conf.ptr[scval];
    
    return retval; 
}

#if 0
long
sysgetclsize(void)
{
    long retval = sys.clsize;

    return retval;
}
#endif

