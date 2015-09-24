#ifndef __KERN_IO_DEV_H__
#define __KERN_IO_DEV_H__

#include <zero/cdecl.h>

#define devop3(dev, op, arg1, arg2, arg3, error)                        \
    (((dev)->ops->op)                                                   \
     ? ((dev)->ops->op(arg1, arg2, arg3))                               \
     : (error))

/* bits for the flg-member */
#define DEV_BLKIO   0x00000001
#define DEV_CANSEEK 0x00000002
struct iodev {
    long  type;         // CDEV, BDEV, ...
    long  id;           // kernel device/object-ID
    long  major;        // major device number
    long  minor;        // minor device number
    char *path;         // device path
    long  flg;          // device flags; DEV_BLKIO, DEV_CANSEEK, ...
    long  nport;        // # of used I/O ports (IDs in portids)
    long *portmap;      // I/O port IDs in use
    long *permbits;     // I/O port permission bitmap
    long  data[EMPTY];  // placeholder [for portmap]
};

#endif /* __KERN_IO_DEV_H__ */

