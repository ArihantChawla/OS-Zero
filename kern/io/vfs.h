#ifndef __KERN_VFS_H__
#define __KERN_VFS_H__

#define VFSDEVNULL 0x00         // /dev/null
#define VFSDEVZERO 0x01         // /dev/zero
#define VFSDEVPTMX 0x02         // /dev/ptmx; POSIX pseudo-teletype multiplexer
#define VFSIPCSHM  0x03         // /ipc/shm/<key>.<virtadr>.<size>
#define VFSIPCRWLK 0x04         // /ipc/rwlk/<devid>.<offset>.<size>.[rd|rw]
#define VFSIPCSEM  0x05         // /ipc/sem/<semid>; on-disk semaphores
#define VFSIPCMQ   0x06         // /ipc/mq/<mqid>; on-disk message queue

#endif /* __KERN_VFS_H__ */

