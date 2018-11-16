#ifndef __KERN_PERM_H__
#define __KERN_PERM_H__

#include <sys/types.h>

/* unix-like permissions */
#define PERM_READ   0x00000001
#define PERM_WRITE  0x00000002
#define PERM_EXEC   0x00000004
#define PERM_OX     0x00000001
#define PERM_OW     0x00000002
#define PERM_OR     0x00000004
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
#define PERM_SYN    0x00010000  // permission for synchronous I/O
#define PERM_BUF    0x00020000  // permission to use buffer-cache
#define PERM_SHM    0x00040000  // permission to share memory
#define PERM_WIRE   0x00080000  // permission to wire memory
//#define PERM_RAWIO  0x20000000   // permission to do raw device I/O operations

/* set permission object elements */
#define setperm(perm, usr, grp, flg)                                    \
    ((perm)->uid = (usr), (perm)->gid = (grp), (perm)->flg = (flg))
#define permchk(perm, usr, grp, type)                                   \
    (!(usr)                                                             \
     ? 1                                                                \
     : (((usr) == (perm)->uid && ((perm)->flg & ((type) << 6)))         \
        ? 1                                                             \
        : (((grp) == (perm)->gid && ((perm)->flg & ((type) << 3)))      \
           ? 1                                                          \
           : ((perm)->flg & ((perm)->flg & (type))))))
#define permread(perm, usr, grp)  permchk(perm, usr, grp, PERM_READ)
#define permwrite(perm, usr, grp) permchk(perm, usr, grp, PERM_WRITE)
#define permexec(perm, usr, grp)  permchk(perm, usr, grp, PERM_EXEC)
#define permsticky(perm)          ((perm)->flg & PERM_STICKY)
#define permsetgid(perm)          ((perm)->flg & PERM_SETGID)
#define permsetuid(perm)          ((perm)->flg & PERM_SETUID)
#define permwire(perm)            ((perm)->flg & PERM_WIRE)
#define permshmap(perm)           ((perm)->flg & PERM_SHMAP)
#define permrawio(perm)           ((perm)->flg & PERM_RAWIO)

struct perm {
    uid_t    uid; // user ID
    gid_t    gid; // group ID
    taskid_t tid; // task ID
    long     flg; // access bits
};

#endif /* __KERN_PERM_H__ */

