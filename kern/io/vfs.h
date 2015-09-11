#ifndef __KERN_VFS_H__
#define __KERN_VFS_H__

#include <kern/list.h>
#include <kern/io/dc.h>

#define VFS_NNESTLINK 8

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

struct openintent {
    long         flg;
    long         creatmode;
    struct file *file;
};

struct vfspath {
    struct vfsmount *mnt;
    struct dcent    *dcent;
};

struct inodedata {
    struct vfspath  path;
    struct dcstr    last;
    struct vfspath  root;
    struct inode   *inode;
    long            flg;
    long            seq;
    long            lasttype;
    long            depth;
    char           *names[FS_NNESTLINK];
    union {
        struct openintent open;
    } intent;
};

struct supblkops {
    struct inode (*ialloc)(struct supblk *);
    void         (*ifree)(struct inode *);
    void         (*idirty)(struct inode *, long);
    long         (*iwrite)(struct inode *, long); // long sync
    void         (*iclear)(struct inode *);
    void         (*idrop)(struct inode *); // LOCK
    void         (*idel)(struct inode *);
    void         (*sbput)(struct sd *); // LOCK
    long         (*fssync)(struct supblk *sb, long); // long wait
    long         (*fsfreeze)(struct supblk *sb);
    long         (*fsunfreeze)(struct supblk *sb);
    long         (*fsstat)(struct dcent *, struct fsstat *);
    long         (*fsremnt)(struct supblk *, long *, char *); // LOCK
    void         (*umntbegin)(struct supblk *sb);
    long         (*showopts)(struct seqfile *, struct dcent *);
    ssize_t      (*qtaread)(struct supblk *, long, char *,
                            size_t, loff_t);
    ssize_t      (*qtawrite)(struct supblk *, long, const char *,
                             size_t, loff_t);
    ssize_t      (*cntmemobj)(struct supblk *sb, long);
    void         (*freememobj)(struct supblk *sb, long);
};

struct inodeops {
    
};

#endif /* __KERN_VFS_H__ */

