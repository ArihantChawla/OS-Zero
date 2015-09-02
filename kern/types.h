#ifndef __KERN_TYPES_H__
#define __KERN_TYPES_H__

#include <stdint.h>
#if defined(__i386__) && !defined(__x86_64__) && !defined(__amd64__)
#include <kern/unit/ia32/types.h>
#elif defined(__x86_64__) || defined(__amd64__)
#include <kern/unit/x86-64/types.h>
#endif

typedef intptr_t desc_t;

#endif /* __KERN_TYPES_H__ */

