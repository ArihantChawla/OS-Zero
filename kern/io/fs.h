#ifndef __KERN_IO_FS_H__
#define __KERN_IO_FS_H__

/* REFERENCE: https://www.kernel.org/doc/Documentation/filesystems/vfs.txt */

#include <zero/spinrw.h>
#include <kern/io/obj.h>

#define FS_NNESTLINK 8

struct openintent {
    long         flg;
    long         creatmode;
    struct file *file;
};

struct dcstr {
    uint32_t             hash;
    uint32_t             len;
    const unsigned char *name;
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

struct dcops {
    long (*isvalid)(struct dcent *ent, struct inodedata *);
    long (*hash)(struct dcent *, struct dcstr *);
    long (*cmp)(struct dcent *, struct dcstr *, struct dcstr *); // LOCK
    long (*del)(struct dcent *);
    void (*free)(struct dcent *);
    void (*iput)(struct dcent *, struct inode *);
    char *(*name)(struct dcent *, char *, long);
};

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

#endif /* __KERN_IO_FS_H__ */

