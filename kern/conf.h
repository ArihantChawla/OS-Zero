#ifndef __KERN_CONF_H__
#define __KERN_CONF_H__

#define ZEROSCHED 1     // default scheduler

#define SMP       0     // enable multiprocessor support; currently broken
#if (SMP)
#define NCPU      8
#else
#define NCPU      1
#endif
#define NPROCFD   32768
#define NPAGEPHYS (16ULL * 1024 * 1024)
#define NBPHYS    (NPAGEPHYS * PAGESIZE)

#define GFXWIDTH  1024
#define GFXHEIGHT 768
#define GFXDEPTH  24

#define BOCHS     1

#define HZ        250

#define DEVEL     1     // debugging

#define NPROC     256   // maximum number of running processes
#define NTHR      4096  // maximum number of running threads

/* planned drivers */
#define PS2DRV    1     // enable PS/2 mouse and keyboard drivers
#define VBE2      0
#define VGAGFX    0     // VGA graphics driver
#define SB16      0     // Soundblaster 16 audio driver
#define AC97      0     // AC97 audio drivers
#define ENS1370   0     // Ensoniq 1370 audio driver

#endif /* __KERN_CONF_H__ */

