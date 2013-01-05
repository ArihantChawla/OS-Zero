#ifndef __UNIT_IA32_KERN_H__
#define __UNIT_IA32_KERN_H__

#include <stdint.h>

void seginit(long core);
void vminit(void *pagetab);
void trapinit(void);
void meminit(uintptr_t base, unsigned long nbphys);
void vgainitcon(int w, int h);
void kbdinit(void);
void mouseinit(void);
void logoprint(void);
void taskinit(void);
void tssinit(long id);

#endif /* __UNIT_IA32_KERN_H__ */

