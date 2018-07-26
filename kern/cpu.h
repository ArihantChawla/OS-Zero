#ifndef __KERN_CPU_H__
#define __KERN_CPU_H__

#include <kern/conf.h>

#if (defined(__i386__) || defined(__i486__)                             \
     || defined(__i586__) || defined(__i686__)                          \
     || defined(__x86_64__) || defined(__amd64__))
#include <kern/unit/x86/cpuid.h>
#include <kern/unit/x86/cpu.h>
#endif

extern volatile struct cpu k_cputab[NCPU];

#endif /* __KERN_CPU_H__ */

