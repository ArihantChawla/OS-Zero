#ifndef __UNIT_IA32_SEG_H__
#define __UNIT_IA32_SEG_H__

#include <kern/unit/x86/seg.h>

#define SEGDEFBITS (SEG32BIT | SEG4KGRAN | SEGPRES)
#define SEGTSS     (SEGAVAILTSS | SEGUSER | SEGPRES)

#endif /* __UNIT_IA32_SEG_H__ */

