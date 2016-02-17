#ifndef __KERN_UNIT_X86_VM_H__
#define __KERN_UNIT_X86_VM_H__

#define VM_FREELIST_DEFAULT 0   // default pages
#define VM_FREELIST_ISADMA  1   // pages below 16 megs for ISA DMA
#define VM_FREELIST_DMA32   2   // pages below 4 gigs not available for ISA DMA
#define VM_DMA32_NPAGE_THRESHOLD 16777216
#define VM_NFREEORDER       13
#define VM_NRESERVELVL      1   // superpage reservations: 1 level
#define VM_LEVEL_0_ORDER    9   // level 0 reservations: 512 pages

#if ((defined(__i386__) || defined(__i486__)                            \
      || defined(__i586__) || defined(__i686__))                        \
     && !defined(__x86_64__) && !defined(__amd64__))
#include <kern/unit/ia32/vm.h>
#else
#include <kern/unit/x86-64/vm.h>
#endif

#endif /* __KERN_UNIT_X86_VM_H__ */

