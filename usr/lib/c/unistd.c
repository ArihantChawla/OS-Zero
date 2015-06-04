#include <kern/conf.h>
#include <errno.h>
#include <features.h>
#include <limits.h>
#include <unistd.h>
#include <sys/sysinfo.h>
#include <zero/cdecl.h>
#include <zero/param.h>
#include <zero/mtx.h>
#include <kern/io/buf.h>
#include <kern/unit/x86/cpu.h>
#if (TEST)
#include <stdio.h>
#include <stdlib.h>
#endif

#define SYSCONF_CPUPROBE 0x00000001
#define SYSCONF_INIT     0x00000002

long sysconftab[NSYSCONF]
= {
    BUFSIZE,                    /* _SC_BLKSIZE */
    0,                          /* _SC_L2NWAY */
    0,                          /* _SC_L2SIZE */
    0,                          /* _SC_L1NDATAWAY */
    0,                          /* _SC_L1NINSTWAY */
    0,                          /* _SC_L1INSTSIZE */
    0,                          /* _SC_L1DATASIZE */
    0,                          /* _SC_CACHELINESIZE */
    0,                          /* _SC_NPROCESSORS_ONLN */
    0,                          /* _SC_NPROCESSORS_CONF */
    0,                          /* _SC_AVPHYS_PAGES */
    0,                          /* _SC_PHYS_PAGES */
    ZERO_VERSION,               /* _SC_OS_VERSION */ // origin for indexing
    _POSIX_C_SOURCE,            /* _SC_VERSION */
    ARG_MAX,                    /* _SC_ARG_MAX */
    CHILD_MAX,                  /* _SC_CHILD_MAX */
    HOST_NAME_MAX,              /* _SC_HOST_NAME_MAX */
    LOGIN_NAME_MAX,             /* _SC_LOGIN_NAME_MAX */
    HZ,                         /* _SC_CLK_TCK */
    OPEN_MAX,                   /* _SC_OPEN_MAX */
    PAGESIZE,                   /* _SC_PAGESIZE aka _SC_PAGE_SIZE */
    RE_DUP_MAX,                 /* _SC_RE_DUP_MAX */
    STREAM_MAX,                 /* _SC_STREAM_MAX */
    SYMLOOP_MAX,                /* _SC_SYMLOOP_MAX */
    TTY_NAME_MAX,               /* _SC_TTY_NAME_MAX */
    TZNAME_MAX                  /* _SC_TZNAME_MAX */
};
volatile long sysconfbits;
volatile long sysconflk;

static void
sysconfinit(long *tab)
{
    struct m_cpuinfo  cpuinfo;
    long             *ptr = tab + NNEGSYSCONF;

    mtxlk(&sysconflk);
    if (!(sysconfbits & SYSCONF_CPUPROBE)) {
        cpuprobe(&cpuinfo);
        ptr[_SC_L2NWAY] = cpuinfo.l2.nway;
        ptr[_SC_L2SIZE] = cpuinfo.l2.size;
        ptr[_SC_L1DATANWAY] = cpuinfo.l1d.nway;
        ptr[_SC_L1INSTNWAY] = cpuinfo.l1i.nway;
        ptr[_SC_L1DATASIZE] = cpuinfo.l1d.size;
        ptr[_SC_L1INSTSIZE] = cpuinfo.l1i.size;
        ptr[_SC_CACHELINESIZE] = cpuinfo.l2.clsz;
        sysconfbits |= SYSCONF_CPUPROBE;
    }
    if (sysconfbits & SYSCONF_INIT) {
        mtxunlk(&sysconflk);

        return;
    }
#if 0
    ptr[_SC_NPROCESSORS_ONLN] = get_nprocs();
    ptr[_SC_NPROCESSORS_CONF] = get_nprocs_conf();
    ptr[_SC_AVPHYS_PAGES] = get_avphys_pages();
    ptr[_SC_PHYS_PAGES] = get_phys_pages();
#endif
    sysconfbits |= SYSCONF_INIT;
    mtxunlk(&sysconflk);

    return;
}

#define _sysconfneedupd(name)                                           \
    ((name) <= _SC_PHYS_PAGES && (name) >= _SC_NPROCESSORS_ONLN)
void
sysconfupd(void)
{
    long *ptr = sysconftab + NNEGSYSCONF;
    
    ptr[_SC_NPROCESSORS_ONLN] = get_nprocs();
    ptr[_SC_NPROCESSORS_CONF] = get_nprocs_conf();
    ptr[_SC_AVPHYS_PAGES] = get_avphys_pages();
    ptr[_SC_PHYS_PAGES] = get_phys_pages();
}

long
sysconf(int name)
{
    long *ptr = sysconftab + NNEGSYSCONF;
    long  retval = -1;

    if (!(sysconfbits & SYSCONF_INIT)) {
        sysconfinit(sysconftab);
    }
    if (_sysconfneedupd(name)) {
        sysconfupd();
    }
    if (name < -NNEGSYSCONF || name > NSYSCONF - NNEGSYSCONF) {
        errno = EINVAL;
    } else {
        retval = ptr[name];
    }
    if (!retval) {
        retval = -1;
    }

    return retval;
}

#if (!_POSIX_SOURCE)

int
getpagesize(void)
{
    int retval;

    if (!(sysconfbits & SYSCONF_INIT)) {
        sysconfinit(sysconftab);
    }
#if defined(PAGESIZE)
    retval = PAGESIZE;
#elif defined(_SC_PAGESIZE)
    retval = sysconf(_SC_PAGESIZE);
#elif defined(_SC_PAGE_SIZE)
    retval = sysconf(_SC_PAGE_SIZE);
#endif

    return retval;
}

#endif /* !_POSIX_SOURCE */

#if (TEST)
int
main(int argc, char *argv[])
{
    fprintf(stderr, "PAGESIZE\t%ld\n", getpagesize());
    fprintf(stderr, "BLKSIZE\t\t%ldK\n", sysconf(_SC_BLKSIZE) >> 10);
    fprintf(stderr, "CLSIZE\t\t%ld\n", sysconf(_SC_CACHELINESIZE));
    fprintf(stderr, "PAGES\t\t%ld\n", sysconf(_SC_PHYS_PAGES));
    fprintf(stderr, "AVPAGES\t\t%ld\n", sysconf(_SC_AVPHYS_PAGES));

    exit(0);
}
#endif

