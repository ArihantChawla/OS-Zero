#ifndef __KERN_CONF_H__
#define __KERN_CONF_H__

#include <zero/param.h>

#define FASTINTR    1

#define NEWFONT     1
#if !defined(BUFNMEG)
#define BUFNMEG     64
#endif

#define HZ          250         // scheduler timer frequency
#define IOAPIC      0
#define SMBIOS      1
#define APIC        1           // local [per-CPU] APIC support
#define SMP         0           // enable multiprocessor support; FIXME:  broken
#define EVHZ        1000        // event timer frequency
#define ZEROSCHED   1           // default scheduler
#define NPROC       1024        // maximum number of running processes on system
#define NTASK       4096        // maximum number of running tasks on system
#define TASKNDESC   16384       // maximum number of descriptors per process
#define TASKSTKSIZE (1024 * 1024)
#define KERNSTKSIZE (4 * PAGESIZE)
#if (SMP)
#define NCPU        8
#else
#define NCPU        1
#endif
#define NPROCFD     4096        // maximum number of descriptors per process
#if !defined(NPAGEPHYS)
//#define NPAGEPHYS   (1ULL * 1024 * 1024) // maximum number of physical pages
#define NPAGEPHYS   (512 * 1024UL) // maximum number of physical pages
#endif
#define NBPHYS      (NPAGEPHYS * PAGESIZE) // maximum amount of RAM

#define GFXWIDTH    1024        // horizontal screen resolution
#define GFXHEIGHT   768         // vertical screen resolution
#define GFXDEPTH    24          // bits per pixel

/* NOTE: it's not recommended to edit anything below unless you develop Zero */

#define LOCORE      0
#define NCONS       8

#define BUFMULTITAB 0

#define BOCHS       1
#define DEVEL       0           // debugging

#define PLASMA      0

/* planned drivers */
#define PCI         1           // PCI bus driver
#define ATA         0           // ATA/ATAPI/SATA/SATAPI
#define ACPI        1           // ACPI
#define ACPICA      0           // ACPICA
#define HPET        1           // high precision event timer support
#define PS2DRV      1           // enable PS/2 mouse and keyboard drivers
#define KBDUS       1           // PS/2 keyboard with US keymap
#define PS2KBDSET2  1
#define VBE         1           // VBE2 graphics driver
#define VGAGFX      0           // VGA graphics driver
#define SB16        0           // Soundblaster 16 audio driver
#define AC97        0           // AC97 audio drivers
#define ENS1370     0           // Ensoniq 1370 audio driver

#endif /* __KERN_CONF_H__ */

