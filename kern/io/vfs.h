#ifndef __KERN_VFS_H__
#define __KERN_VFS_H__

#include <kern/list.h>

#define VFSDEVNULL 0x00         // /dev/null
#define VFSDEVZERO 0x01         // /dev/zero
#define VFSDEVPTMX 0x02         // /dev/ptmx; POSIX pseudo-teletype multiplexer
#define VFSIPCSHM  0x03         // /ipc/shm/<key>.<virtadr>.<size>
#define VFSIPCRWLK 0x04         // /ipc/rwlk/<devid>.<offset>.<size>.[rd|rw]
#define VFSIPCSEM  0x05         // /ipc/sem/<semid>; on-disk semaphores
#define VFSIPCMQ   0x06         // /ipc/mq/<mqid>; on-disk message queue

struct vfsmnt {
    long           flg;
    char          *devname;
    struct vfsmnt *hash;
    struct vfsmnt *parent;
    struct dcent  *dcent;
    struct dcent  *root;
    struct supblk *sb;
    struct vfsmnt *childlist;
    struct vfsmnt *mntlist;
    struct vfsmnt *expire;
    struct vfsmnt *share;
    struct vfsmnt *slave;
    struct vfsmnt *slavelist;
};

#endif /* __KERN_VFS_H__ */

