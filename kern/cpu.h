#ifndef __KERN_CPU_H__
#define __KERN_CPU_H__

#if (defined(__i386__) || defined(__i486__)                             \
     || defined(__i586__) || defined(__i686__)                          \
     || defined(__x86_64__) || defined(__amd64__))
#include <kern/unit/x86/cpu.h>
#endif

#endif /* __KERN_CPU_H__ */

