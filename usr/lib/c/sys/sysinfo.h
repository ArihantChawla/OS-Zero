#ifndef __SYS_SYSINFO_H__
#define __SYS_SYSINFO_H__

#include <features.h>

/* TODO: struct sysinfo, sysinfo() */

#if (_GNU_SOURCE)
extern int  get_nprocs_conf(void);
extern int  get_nprocs(void);
extern long get_phys_pages(void);
extern long get_avphys_pages(void);
#endif

/* structure from linux */
struct sysinfo {
    long           uptime;    /* Seconds since boot */
    unsigned long  loads[3];  /* 1, 5, and 15 minute load averages */
    unsigned long  totalram;  /* Total usable main memory size */
    unsigned long  freeram;   /* Available memory size */
    unsigned long  sharedram; /* Amount of shared memory */
    unsigned long  bufferram; /* Memory used by buffers */
    unsigned long  totalswap; /* Total swap space size */
    unsigned long  freeswap;  /* swap space still available */
    unsigned short procs;     /* Number of current processes */
    unsigned long  totalhigh; /* Total high memory size */
    unsigned long  freehigh;  /* Available high memory size */
    unsigned int   mem_unit;  /* Memory unit size in bytes */
};

#endif /* __SYS_SYSINFO_H__ */

