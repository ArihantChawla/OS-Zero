#ifndef __KERN_IO_FILE_H__
#define __KERN_IO_FILE_H__

#include <stdint.h>
#include <fcntl.h>
#include <sys/types.h>
#include <kern/crec.h>

#define DTYPE_VNODE 1
#define DTYPE_SOCK  2
#define DTYPE_PIPE  3
#define DTYPE_FIFO  4
#define DTYPE_EVQ   5
#define DTYPE_CRYPT 6
#define DTYPE_MQ    7
#define DTYPE_SHM   8
#define DTYPE_SEM   9
#define DTYPE_PTS   10
#define DTYPE_DEV   11
#define DTYPE_PROC  12

typedef long file_rdwr_func(struct file *fp, struct uio *uio,
                            struct cred *ucred, long flg,
                            struct thr *thr);
typedef long file_truncate_func(struct file *fp, off_t len,
                                struct cred *ucred, struct thr *thr);
typedef long file_ioctl_func(struct file *fp, long cmd, void *data,
                             struct cred *ucred, struct thr *thr);
typedef long file_poll_func(struct file *file, long events,
                            strct cred *ucred, struct thr *thr);
typedef long file_stat_func(struct file *fp, struct stat *sb,
                            struct cred *ucred, struct thr *thr);
typedef long file_close_func(struct file *fp, struct thr *thr);
typedef long file_chmod_func(struct file *fp, mode_t mode,
                             struct cred *ucred, struct thr *thr);
typedef long file_chown_func(struct file *fp, uid_t uid, gid_t gid,
                             struct cred *ucred, struct thr *thr);
typedef long file_seek_func(struct file *fp, off_t ofs, long whence,
                            struct thr *thr);
typedef long file_mmap_func(struct file *fp, struct vmmap *map, uintptr_t *adr,
                            uintptr_t size, long prot, long maxprot,
                            long flg, intptr_t ofs, struct thr *thr);

/* TODO: sendfile(), kqfilter(), fill_kinfo() */

/* flg values */
#define DFLAG_PASSABLE 0x01     // may be passed via Unix sockets
#define DFLAG_SEEKABLE 0x02     // seekable / nonsequential
struct fileops {
    file_rdwr_func     *read;
    file_rdwr_func     *write;
    file_truncate_func *truncate;
    file_ioctl_func    *ioctl;
    file_poll_func     *poll;
    file_stat_func     *stat;
    file_close_func    *close;
    file_chmod_func    *chmod;
    file_chown_func    *chown;
    file_seek_func     *seek;
    file_mmap_func     *mmap;
    long                flg;
};

struct fadvise_info {
    long  fa_advice;    // FADV_* type
    off_t fa_start;     // region start
    off_t fa_end;       // region end
    off_t fa_prevstart; // previous NOREUSE start
    off_t fa_prevend;   // previous NOREUSE end
};

struct file {
    void           *data;       // file descriptor specific data
    struct fileops  fileops;    // file operations
    struct cred    *cred;
};
    
#endif /* __KERN_IO_FILE_H__ */

