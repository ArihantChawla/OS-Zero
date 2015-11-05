#ifndef __KERN_PERM_H__
#define __KERN_PERM_H__

/* unix-like permissions */
#define PERM_READ   0x00000001
#define PERM_WRITE  0x00000002
#define PERM_EXEC   0x00000004
#define PERM_OX     0x00000010
#define PERM_OW     0x00000020
#define PERM_OR     0x00000040
#define PERM_GX     0x00000080
#define PERM_GW     0x00000100
#define PERM_GR     0x00000200
#define PERM_UX     0x00000400
#define PERM_UW     0x00000800
#define PERM_UR     0x00001000
#define PERM_STICKY 0x00004000
#define PERM_SETGID 0x00008000
#define PERM_SETUID 0x00010000
/* zero permissions */
#define PERM_WIRE   0x80000000   // permission to wire memory
#define PERM_SHM    0x40000000   // permission to share memory
//#define PERM_RAWIO  0x20000000   // permission to do raw device I/O operations

#define setperm(perm, usr, grp, flg)                                    \
    ((perm)->uid = (usr), (perm)->gid = (grp), (perm)->flg = (flg))
#define permchk(usr, grp, perm, type)                                   \
    (!(usr)                                                             \
     ? 1                                                                \
     : (((usr) == (perm)->uid && ((perm)->flg & ((type) << 6)))         \
        ? 1                                                             \
        : (((grp) == (perm)->gid && ((perm)->flg & ((type) << 3)))      \
           ? 1                                                          \
           : ((perm)->flg & ((perm)->flg & (type))))))
#define permread(usr, grp, perm)  permchk(usr, grp, perm, PERM_READ)
#define permwrite(usr, grp, perm) permchk(usr, grp, perm, PERM_WRITE)
#define permexec(usr, grp, perm)  permchk(usr, grp, perm, PERM_EXEC)
#define permsticky(perm)          ((perm)->flg & PERM_STICKY)
#define permsetgid(perm)          ((perm)->flg & PERM_SETGID)
#define permsetuid(perm)          ((perm)->flg & PERM_SETUID)
#define permwire(perm)            ((perm)->flg & PERM_WIRE)
#define permshmap(perm)           ((perm)->flg & PERM_SHMAP)
#define permrawio(perm)           ((perm)->flg & PERM_RAWIO)

struct perm {
    long        uid;    // user ID
    long        gid;    // group ID
    long        flg;    // access bits
    struct acl *acl;    // access-control [object] list
};

#endif /* __KERN_PERM_H__ */

