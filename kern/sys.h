#ifndef __KERN_SYS_H__
#define __KERN_SYS_H__

#include <kern/version.h>
#include <bits/unistd.h>

/* vals is indexed with _SC-macros from <unistd.h> */
struct sysconf {
    long vals[NSYSCONF];
};

struct sys {
    struct sysconf conf;
    long           clsize;
};

#endif /* __KERN_SYS_H__ */

