#include <kern/conf.h>
#include <kern/version.h>
#include <errno.h>
#include <features.h>
#include <limits.h>
#include <unistd.h>
#include <sys/sysinfo.h>
#include <zero/cdefs.h>
#include <mach/param.h>
#include <mt/mtx.h>
#include <kern/cpu.h>
#include <kern/io/buf.h>
//#include <kern/unit/x86/cpu.h>
#if defined(TESTUNISTD)
#include <stdio.h>
#include <stdlib.h>
#endif

#define SYSCONF_INIT     0x00000001
#define SYSCONF_CPUPROBE 0x00000002

long sysconftab[NSYSCONF]
= {
    BUFBLKSIZE,                 /* _SC_BLKSIZE */
    0,                          /* _SC_L2_NWAY */
    0,                          /* _SC_L2_SIZE */
    0,                          /* _SC_L1NWAYDATA */
    0,                          /* _SC_L1NWAYINST */
    0,                          /* _SC_L1INSTSIZE */
    0,                          /* _SC_L1DATASIZE */
    0,                          /* _SC_CACHELINE_SIZE */
    0,                          /* _SC_NPROCESSORS_ONLN */
    0,                          /* _SC_NPROCESSORS_CONF */
    0,                          /* _SC_AVPHYS_PAGES */
    0,                          /* _SC_PHYS_PAGES */
    _ZERO_VERSION,              /* _SC_OS_VERSION */ // origin for indexing
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
zerofmtx      sysconflk;
struct cpu    sysconfcpu;

#define _sysconfneedupd(name)                                           \
    ((name) <= _SC_PHYS_PAGES && (name) >= _SC_NPROCESSORS_ONLN)

long
sysconfupd(int name)
{
    long *ptr = sysconftab - MINSYSCONF;
    long  ret = -1;

    switch (name) {
        case _SC_NPROCESSORS_ONLN:
            ret = ptr[_SC_NPROCESSORS_ONLN] = get_nprocs();

            break;
        case _SC_NPROCESSORS_CONF:
            ret = ptr[_SC_NPROCESSORS_CONF] = get_nprocs_conf();

            break;
        case _SC_AVPHYS_PAGES:
            ret = ptr[_SC_AVPHYS_PAGES] = get_avphys_pages();

            break;
        case _SC_PHYS_PAGES:
            ret = ptr[_SC_PHYS_PAGES] = get_phys_pages();

            break;
    }

    return ret;
}

static void
sysconfinit(long *tab)
{
    long       *ptr = tab - MINSYSCONF;
    struct cpu *cpu = &sysconfcpu;

    fmtxlk(&sysconflk);
    if (!(sysconfbits & SYSCONF_CPUPROBE)) {
        /* probe persistent values */
        cpuprobe(cpu);
        ptr[_SC_L2_NWAY] = cpu->info.cache.l2.nway;
        ptr[_SC_L2_SIZE] = cpu->info.cache.l2.size;
        ptr[_SC_L1_DATA_NWAY] = cpu->info.cache.l1d.nway;
        ptr[_SC_L1_INST_NWAY] = cpu->info.cache.l1i.nway;
        ptr[_SC_L1_DATA_SIZE] = cpu->info.cache.l1d.size;
        ptr[_SC_L1_INST_SIZE] = cpu->info.cache.l1i.size;
        ptr[_SC_CACHELINE_SIZE] = cpu->info.cache.l2.clsz;
        sysconfbits |= SYSCONF_CPUPROBE;
    }
    if (sysconfbits & SYSCONF_INIT) {
        fmtxunlk(&sysconflk);

        return;
    }
//    sysconfupd();
    sysconfbits |= SYSCONF_INIT;
    fmtxunlk(&sysconflk);

    return;
}

long
sysconf(int name)
{
    long *ptr = sysconftab - MINSYSCONF;
    long  retval = -1;

    if (!(sysconfbits & SYSCONF_INIT)) {
        sysconfinit(sysconftab);
    }
    if (_sysconfneedupd(name)) {
        retval = sysconfupd(name);
    }
    if (name < MINSYSCONF || name > NSYSCONF + MINSYSCONF) {
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

#if defined(TESTUNISTD)
int
main(int argc, char *argv[])
{
    fprintf(stderr, "PAGESIZE\t%ld\n", getpagesize());
    fprintf(stderr, "BLKSIZE\t\t%ldK\n", sysconf(_SC_BLKSIZE) >> 10);
    fprintf(stderr, "CLSIZE\t\t%ld\n", sysconf(_SC_CACHELINE_SIZE));
    fprintf(stderr, "PAGES\t\t%ld\n", sysconf(_SC_PHYS_PAGES));
    fprintf(stderr, "AVPAGES\t\t%ld\n", sysconf(_SC_AVPHYS_PAGES));

    exit(0);
}
#endif

