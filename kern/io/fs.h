#ifndef __KERN_IO_FS_H__
#define __KERN_IO_FS_H__

/* REFERENCE: https://www.kernel.org/doc/Documentation/filesystems/vfs.txt */

#include <zero/spinrw.h>
#include <kern/io/obj.h>

/* type */
#define FSVFS 0
/* flg-values */
#define FS_NO_DIRCACHE (1 << 0)
/* struct dcent * mnt(struct fs *fs, long flg, const char *dev, void *opts); */
struct fs {
    struct mod      *mod;        // owner module
    const char      *name;       // e.g., VFS
    long             type;       // e.g. FSVFS
    long             flg;
    struct supblk *(*sbget)(struct fs *, long, char *, void *);
    void           (*sbkill)(struct supblk *);
    struct fs       *next;
    struct supblk   *sblist;
    struct spinrw    sblk;
    struct spinrw    umntlk;
    struct spinrw    ilk;
    volatile long    imtx;
    volatile long    imtxdir;
    struct sem       iallocsem;
};

#endif /* __KERN_IO_FS_H__ */

