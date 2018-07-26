#ifndef __KERN_SYS_H__
#define __KERN_SYS_H__

#include <unistd.h>

/* vals is indexed with _SC-macros from <unistd.h> */
struct sysconf {
    long  tab[NSYSCONF];
    long *ptr;
};

struct sys {
    struct sysconf conf;
};

#endif /* __KERN_SYS_H__ */

