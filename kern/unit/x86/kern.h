#ifndef __UNIT_X86_KERN_H__
#define __UNIT_X86_KERN_H__

#include <kern/conf.h>

extern void trapinit(void);
extern void seginit(long core);
extern void vminit(void *pagetab);
extern void picinit(void);
extern void meminit(unsigned long nbphys, unsigned long nbvirt);
extern void vgainitcon(int w, int h);
extern void kbdinit(void);
extern void mouseinit(void);
extern void logoprint(void);
extern void tssinit(long id);
extern void cpuinit(long unit);
extern long sysinit(void);
extern long ioinitbuf(void);
#if (VBE)
extern void trapinitprot(void);
#endif
#if (HPET)
extern void hpetinit(void);
#endif
#if (SMP)
extern void mpstart(void);
#endif
#if (VBE)
extern void idtinit(uint64_t *idt);
extern void vbeinitscr(void);
extern void vbeprintinfo(void);
#endif
#if (SMBIOS)
extern void smbiosinit(void);
#endif
#if (PS2DRV)
extern void ps2init(void);
#endif
#if (PLASMA)
extern void plasmaloop(long nsec);
#endif
#if (PCI)
extern void pciinit(void);
#endif
#if (ATA)
extern void atainit(void);
#endif
#if (AC97)
extern long ac97init(void);
#endif
#if (ACPI)
extern void acpiinit(void);
#endif
#if (SB16)
extern void sb16init(void);
#endif
#if (APIC)
extern void apicinit(void);
extern void apicstarttmr(void);
#endif
extern void taskinitenv(void);
#if (USERMODE)
extern FASTCALL void m_jmpusr(long id, void *func);
#endif
extern void taskinit(struct task *task, long unit);
extern void schedloop(void);

#endif /* __UNIT_X86_KERN_H__ */

