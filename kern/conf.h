#ifndef __KERN_CONF_H__
#define __KERN_CONF_H__

#define BUFSIZE     (1UL << BUFSIZELOG2)
#define BUFSIZELOG2 12                          // TODO: make this per-device

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
#define NPAGEPHYS (16ULL * 1024 * 1024)         // maximum number of physical pages
#define NBPHYS    (NPAGEPHYS * PAGESIZE)        // maximum amount of RAM

#define GFXWIDTH  1024                          // horizontal screen resolution
#define GFXHEIGHT 768                           // vertical screen resolution
#define GFXDEPTH  24                            // bits per pixel

/* NOTE: it's not recommended to edit anything below unless you develop Zero */

#define BOCHS     1
#define DEVEL     1     // debugging

/* planned drivers */
#define HPET      1     // high precision event timer support
#define PS2DRV    1     // enable PS/2 mouse and keyboard drivers
#define VBE2      0     // VBE2 graphics driver
#define VGAGFX    0     // VGA graphics driver
#define SB16      0     // Soundblaster 16 audio driver
#define AC97      0     // AC97 audio drivers
#define ENS1370   0     // Ensoniq 1370 audio driver

#endif /* __KERN_CONF_H__ */

