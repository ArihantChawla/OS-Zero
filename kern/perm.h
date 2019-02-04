#ifndef __KERN_PERM_H__
#define __KERN_PERM_H__

#include <sys/types.h>

/* unix-like permissions */
#define PERM_EXEC   0x00000001
#define PERM_WRITE  0x00000002
#define PERM_READ   0x00000004
#define PERM_AX     0x00000001
#define PERM_AW     0x00000002
#define PERM_AR     0x00000004
#define PERM_GX     0x00000008
#define PERM_GW     0x00000010
#define PERM_GR     0x00000020
#define PERM_UX     0x00000040
#define PERM_UW     0x00000080
#define PERM_UR     0x00000100
#define PERM_STICKY 0x00000200
#define PERM_SETGID 0x00000400
#define PERM_SETUID 0x00000800
/* zero permissions */
#define PERM_RAW    0x00010000  // permission for raw [unbuffered] I/O
#define PERM_SYNC   0x00020000  // permission for synchronous I/O
#define PERM_BUF    0x00040000  // permission to use buffer-cache
#define PERM_MAP    0x00080000  // permission to map/share memory
#define PERM_WIRE   0x00100000  // permission to wire memory
//#define PERM_RAWIO  0x20000000   // permission to do raw device I/O operations

/* set permission object elements */
#define permset(perm, task, usr, grp, flg)                              \
    ((perm)->pid = (task),                                              \
     (perm)->uid = (usr),                                               \
     (perm)->gid = (grp),                                               \
     (perm)->flg = (flg))
/* check for PERM_EXEC, PERM_READ, PERM_WRITE */
#define permchk(perm, usr, grp, type)                                   \
    (!(usr)                                                             \
     ? 1                                                                \
     : (((usr) == (perm)->uid && ((perm)->flg & ((type) << 6)))         \
        ? 1                                                             \
        : (((grp) == (perm)->gid && ((perm)->flg & ((type) << 3)))      \
           ? 1                                                          \
           : ((perm)->flg & ((perm)->flg & (type))))))
#define permchkread(perm, usr, grp)  permchk(perm, usr, grp, PERM_READ)
#define permchkwrite(perm, usr, grp) permchk(perm, usr, grp, PERM_WRITE)
#define permchkexec(perm, usr, grp)  permchk(perm, usr, grp, PERM_EXEC)
#define permchkmap(perm, usr, grp)                                      \
    (permchkexec(perm, usr, grp) && permismap(perm))
#define permissticky(perm)           ((perm)->flg & PERM_STICKY)
#define permissetgid(perm)           ((perm)->flg & PERM_SETGID)
#define permissetuid(perm)           ((perm)->flg & PERM_SETUID)
#define permisraw(perm)              ((perm)->flg & PERM_RAW)
#define permissync(perm)             ((perm)->flg & PERM_SYNC)
#define permisbuf(perm)              ((perm)->flg & PERM_BUF)
#define permismap(perm)              ((perm)->flg & PERM_MAP)
#define permiswire(perm)             ((perm)->flg & PERM_WIRE)

struct perm {
    pid_t pid; // task ID
    uid_t uid; // user ID
    gid_t gid; // group ID
    long  flg; // access bits
};

#endif /* __KERN_PERM_H__ */

