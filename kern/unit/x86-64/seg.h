#ifndef __UNIT_X86_64_SEG_H__
#define __UNIT_X86_64_SEG_H__

#include <stdint.h>
#include <kern/unit/x86/trap.h>

#define SEGDEFBITS  (SEG64BIT | SEG4KGRAN | SEGPRES)
#define SEGTSS      (SEGAVAILTSS | SEGUSER | SEGPRES | SEGISTMASK)

#endif /* __UNIT_X86_64_SEG_H__ */

