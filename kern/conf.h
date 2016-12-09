#ifndef __KERN_CONF_H__
#define __KERN_CONF_H__

#include <zero/param.h>

#define REENTRANTGDTINIT 1
#define LONGMODE      0
#define MEMBUFNOLK    0 // FIXME: implement lock-free data (stack?)

#define MEMPARANOIA   0
#define BUFDYNALLOC   0
#define MEMNBUFBLK    8192
#define PERTHRSTACKS  0

#define NEWBUFBLK     1
#define BUFNEWHASH    1
#define BUFMULTITAB   0

#define VMFLATPHYSTAB 1
#define USERMODE      0
#define PLASMA        0
#define PLASMAFOREVER 0

#if !defined(FASTINTR)
#define FASTINTR      1
#endif

#define IOBUF         1
#define NEWFONT       1
#if !defined(BUFNMEG)
#define BUFNMEG       128
#endif
#define BUFNBYTE      (BUFNMEG * 1024 * 1024)
#define BUFMINBYTES   (BUFNBYTE >> 3)

#define kgethz()      HZ
#define HZ            250       // scheduler timer frequency
#define SMP           0         // enable multiprocessor support; FIXME:  broken
#define SMT           0         // enable hardware multithreading ('hyper')
#define FASTHZ        1000      // fast timer frequency (interactive tasks etc.)
#define ZEROSCHED     1         // default scheduler
#define ZEROINTSCHED  1         // scheduler with interactivity-scoring
#define ZEROULE       0
//#define NPROC       1024      // maximum number of running processes on system
//#define NPROC       4096
#define NTASK         4096      // maximum number of running tasks on system
#define NPROCTASK     128
#define TASKNDESC     16384     // maximum number of descriptors per process
#define TASKSTKSIZE   1048576
#define KERNSTKSIZE   PAGESIZE
#if (SMP)
#define NCPU          8
#else
#define NCPU          1
#endif
#if (SMT)
#define NCORE         8
#else
#define NCORE         1
#endif
#define NPROCFD       4096      // maximum number of descriptors per process

/* NOTE: it's not recommended to edit anything below unless you develop Zero */

#define GFXWIDTH      1024      // horizontal screen resolution
#define GFXHEIGHT     768       // vertical screen resolution
#define GFXDEPTH      24        // bits per pixel

/* maximum number of physical pages */
#if (PTRBITS == 32)
#define NPAGEMAX      (1L << (PTRBITS - PAGESIZELOG2))
#endif
#define NPAGEPHYS     (1L << (PTRBITS - PAGESIZELOG2))

#define LOCORE        0
#define NCONS         8

#define BOCHS         1
#define DEVEL         0         // debugging

/* planned and developed drivers */
#define VBE           1         // VBE2 graphics driver
#define VBEMTRR       0         // map memory write-combining
#define PIC           0
#define IOAPIC        0         // enable I/O APIC support
#define SMBIOS        0         // SMBIOS (system management) support
#define APIC          1         // local [per-CPU] APIC support
#define PCI           1         // PCI bus driver
#define ATA           1         // ATA/ATAPI/SATA/SATAPI
#define ACPI          0         // ACPI
#define ACPICA        0         // ACPICA
#define HPET          1         // high precision event timer support
#define PS2DRV        0         // enable PS/2 mouse and keyboard drivers
#define KBDUS         0         // PS/2 keyboard with US keymap
#define PS2KBDSET2    0
#define VGAGFX        0         // VGA graphics driver
#define SB16          0         // Soundblaster 16 audio driver
#define AC97          1         // AC97 audio drivers
#define ENS1370       0         // Ensoniq 1370 audio driver

#endif /* __KERN_CONF_H__ */

