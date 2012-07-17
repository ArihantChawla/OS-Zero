#ifndef __KERN_CONF_H__
#define __KERN_CONF_H__

#define BOCHS   1

#define HZ      250

#define DEVEL   1       // debugging

#define SMP     0       // enable multiprocessor support
#define PS2DRV  1       // enable PS/2 mouse and keyboard drivers

#define NPROC   256     // maximum number of running processes
#define NTHR    4096    // maximum number of running threads

/* planned drivers */
#define SB16    0
#define AC97    0
#define ENS1370 0

#endif /* __KERN_CONF_H__ */

