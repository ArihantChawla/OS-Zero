#ifndef __UNIT_X86_KERN_H__
#define __UNIT_X86_KERN_H__

#include <stdint.h>

void trapinit(void);
void seginit(long core);
void vminit(void *pagetab);
void picinit(void);
void meminit(unsigned long nbphys, unsigned long nbvirt);
void vgainitcon(int w, int h);
void kbdinit(void);
void mouseinit(void);
void logoprint(void);
void taskinit(void);
void tssinit(long id);

#endif /* __UNIT_X86_KERN_H__ */

