#ifndef __KERN_CONF_H__
#define __KERN_CONF_H__

#if defined(__x86_64__)
#include <kern/unit/x86-64/conf.h>
#elif defined(__i386__)
#include <kern/unit/ia32/conf.h>
#elif defined(__arm__)
#include <kern/unit/arm/conf.h>
#elif defined(__ppc__)
#include <kern/unit/ppc/conf.h>
#endif

#define NEWFONT   1

#define APIC      1     // local [per-cpu] APIC support
#define SMP       1     // enable multiprocessor support; currently broken
#define HZ        250   // scheduler timer frequency
#define EVHZ      1000  // event timer frequency
#define ZEROSCHED 1     // default scheduler
#define NPROC     1024  // maximum number of running processes
#define NTHR      4096  // maximum number of running threads
#if (SMP)
#define NCPU      8
#else
#define NCPU      1
#endif
#define NPROCFD   32768 // maximum number of file descriptors per process
#if !defined(NPAGEPHYS)
#define NPAGEPHYS (16ULL * 1024 * 1024) // maximum number of physical pages
#endif
#define NBPHYS    (NPAGEPHYS * PAGESIZE) // maximum amount of RAM
#define NMEGBUF   64

#define GFXWIDTH  1024  // horizontal screen resolution
#define GFXHEIGHT 768   // vertical screen resolution
#define GFXDEPTH  24    // bits per pixel

/* NOTE: it's not recommended to edit anything below unless you develop Zero */

#define NCONS     8

#define BOCHS     1
#define DEVEL     0     // debugging

#define PLASMA    1

/* planned drivers */
#define PCI       1     // PCI bus driver
#define ATA       1     // ATA/ATAPI/SATA/SATAPI
#define ACPI      1     // ACPI
#define ACPICA    0     // ACPICA
#define HPET      1     // high precision event timer support
#define PS2DRV    1     // enable PS/2 mouse and keyboard drivers
#define KBDUS     1     // PS/2 keyboard with US keymap
#define VBE       1     // VBE2 graphics driver
#define VGAGFX    0     // VGA graphics driver
#define SB16      1     // Soundblaster 16 audio driver
#define AC97      1     // AC97 audio drivers
#define ENS1370   0     // Ensoniq 1370 audio driver

#endif /* __KERN_CONF_H__ */

