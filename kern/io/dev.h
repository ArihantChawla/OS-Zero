#ifndef __KERN_IO_DEV_H__
#define __KERN_IO_DEV_H__

#include <zero/cdecl.h>

/* bits for the flg-member */
#define DEV_BLKIO   0x00000001
#define DEV_CANSEEK 0x00000002
struct dev {
    long  id;           // kernel device/object-ID
    long  major;        // major device number
    long  minor;        // minor device number       
    long  flg;          // device flags; DEV_BLKIO, DEV_CANSEEK, ...
    long  nport;        // # of used I/O ports (IDs in portids)
    long *portmap;      // I/O port IDs in use
    long *permbits;     // I/O port permission bitmap
    long  data[EMPTY];  // placeholder [for portmap]
};

#endif /* __KERN_IO_DEV_H__ */

