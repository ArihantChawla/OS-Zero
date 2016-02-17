#ifndef __KERN_VFS_H__
#define __KERN_VFS_H__

#include <zero/ref.h>
#include <zero/mtx.h>
#include <kern/perm.h>
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
    long              flg;
    char             *devname;
    struct vfsmnt    *hash;
    struct vfsmnt    *parent;
    struct dcent     *dcent;
    struct dcent     *root;
    struct vfssupblk *sb;
    struct vfsmnt    *childlist;
    struct vfsmnt    *mntlist;
    struct vfsmnt    *expire;
    struct vfsmnt    *share;
    struct vfsmnt    *slave;
    struct vfsmnt    *slavelist;
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

struct vnodedata {
    struct vfspath  path;
    struct dcstr    last;
    struct vfspath  root;
    struct vnode   *inode;
    long            flg;
    long            seq;
    long            lasttype;
    long            depth;
    char           *names[FS_NNESTLINK];
    union {
        struct openintent open;
    } intent;
};

/* locks and unlocks vnode */
#define vfsrefnode(vp)                                                  \
    do {                                                                \
        mtxlk(&(vp)->lk);                                               \
        refinc(&(vp)->usecnt);                                          \
        mtxunlk(&(vp)->lk);                                             \
    } while (0)
    
/* takes unlocked vnode, does not lock it */
#define vfsunrefnode(vp)                                                \
    do {                                                                \
        refdec(&(vp)->usecnt);                                          \
    } while (0)
    
/* locks and unlocks vnode */
#define vfsrelnode(vp)                                                  \
    do {                                                                \
        mtxlk(&(vp)->lk);                                               \
        refdec(&(vp)->usecnt);                                          \
        mtxunlk(&(vp)->lk);                                             \
    } while (0)

/* takes locked vnode, releases the lock */
#define vfsputnode(vp)                                                  \
    do {                                                                \
        refdec(&(vp)->usecnt);                                          \
        mtxunlk(&(vp)->lk);                                             \
    } while (0)

#define VNODE_NOTYPE 0          // no type (uninitialised)
#define VNODE_REG    1          // regular file
#define VNODE_DIR    2          // directory
#define VNODE_BLK    3          // block device
#define VNODE_CHR    4          // character device
#define VNODE_LNK    5          // symbolic link
#define VNODE_SOCK   6          // socket
#define VNODE_FIFO   7          // named pipe
#define VNODE_BAD    (~0L)      // reclaimed vnode
struct vnode {
    volatile long lk;           // mutual exclusion for modifications
    uintptr_t     id;           // vnode ID (kernel pointer)
    long          type;
    struct perm   perm;
    volatile long usecnt;       // # of user clients
    volatile long holdcnt;      // # of users who veto the recycling of the node
    volatile long writecnt;     // # of writers for file
};

struct vfssupblkops {
    struct vnode (*ialloc)(struct vfssupblk *);
    void         (*ifree)(struct vnode *);
    void         (*idirty)(struct vnode *, long);
    long         (*iwrite)(struct vnode *, long); // long sync
    void         (*iclear)(struct vnode *);
    void         (*idrop)(struct vnode *); // LOCK
    void         (*idel)(struct vnode *);
    void         (*sbput)(struct sd *); // LOCK
    long         (*fssync)(struct vfssupblk *sb, long); // long wait
    long         (*fsfreeze)(struct vfssupblk *sb);
    long         (*fsunfreeze)(struct vfssupblk *sb);
    long         (*fsstat)(struct dcent *, struct fsstat *);
    long         (*fsremnt)(struct vfssupblk *, long *, char *); // LOCK
    void         (*umntbegin)(struct vfssupblk *sb);
    long         (*showopts)(struct seqfile *, struct dcent *);
    ssize_t      (*qtaread)(struct vfssupblk *, long, char *,
                            size_t, loff_t);
    ssize_t      (*qtawrite)(struct vfssupblk *, long, const char *,
                             size_t, loff_t);
    ssize_t      (*cntmemobj)(struct vfssupblk *sb, long);
    void         (*freememobj)(struct vfssupblk *sb, long);
};

struct vnodeops {
    ;
};

#endif /* __KERN_VFS_H__ */

