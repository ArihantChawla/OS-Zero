#ifndef __KERN_SYS_H__
#define __KERN_SYS_H__

#include <kern/version.h>
#include <unistd.h>
//#include <bits/unistd.h>
#if (defined(__i386__) || defined(__i486__)                             \
     || defined(__i586__) || defined(__i686__)                          \
     || defined(__x86_64__) || defined(__amd64__))
#include <kern/unit/x86/cpu.h>
#endif

/* vals is indexed with _SC-macros from <unistd.h> */
struct sysconf {
    long  tab[NSYSCONF];
    long *ptr;
};

struct sys {
    struct sysconf conf;
};

#endif /* __KERN_SYS_H__ */

