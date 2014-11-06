#ifndef __SYS_SYSINFO_H__
#define __SYS_SYSINFO_H__

#include <features.h

/* TODO: struct sysinfo, sysinfo() */

#if (_GNU_SOURCE)
extern int  get_nprocs_conf(void);
extern int  get_nprocs(void);
extern long get_phys_pages(void);
extern long get_avphys_pages(void);
#endif

#endif /* __SYS_SYSINFO_H__ */

