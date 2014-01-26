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

#define BUFSIZE     (1UL << BUFSIZELOG2)
#define BUFSIZELOG2 12                          // TODO: make this per-device

#define SMP       0     // enable multiprocessor support; currently broken
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
#define NPAGEPHYS (16ULL * 1024 * 1024)         // maximum number of physical pages
#endif
#define NBPHYS    (NPAGEPHYS * PAGESIZE)        // maximum amount of RAM

#define GFXWIDTH  1024                          // horizontal screen resolution
#define GFXHEIGHT 768                           // vertical screen resolution
#define GFXDEPTH  24                            // bits per pixel

/* NOTE: it's not recommended to edit anything below unless you develop Zero */

#define BOCHS     1
#define DEVEL     1     // debugging

#define CMOV      1

/* planned drivers */
#define ACPI      1
#define HPET      1     // high precision event timer support
#define PS2DRV    1     // enable PS/2 mouse and keyboard drivers
#define VBE       1     // VBE2 graphics driver
#define VGAGFX    0     // VGA graphics driver
#define SB16      0     // Soundblaster 16 audio driver
#define AC97      0     // AC97 audio drivers
#define ENS1370   0     // Ensoniq 1370 audio driver

#endif /* __KERN_CONF_H__ */

