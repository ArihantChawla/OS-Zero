#ifndef __KERN_PERM_H__
#define __KERN_PERM_H__

/* unix-like permissions */
#define PERMREAD   0x00000001
#define PERMWRITE  0x00000002
#define PERMEXEC   0x00000004
#define PERMOX     0x00000001
#define PERMOW     0x00000002
#define PERMOR     0x00000004
#define PERMGX     0x00000008
#define PERMGW     0x00000010
#define PERMGR     0x00000020
#define PERMUX     0x00000040
#define PERMUW     0x00000080
#define PERMUR     0x00000100
#define PERMSTICKY 0x00000400
#define PERMSETGID 0x00000800
#define PERMSETUID 0x00001000
/* zero permissions */
#define PERMWIRE   0x80000000   // permission to wire memory
#define PERMSHMAP  0x40000000   // permission to share-map memory
#define PERMRAWIO  0x20000000   // permission to do raw device I/O operations

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
#define permread(usr, grp, perm)  permchk(usr, grp, perm, PERMREAD)
#define permwrite(usr, grp, perm) permchk(usr, grp, perm, PERMWRITE)
#define permexec(usr, grp, perm)  permchk(usr, grp, perm, PERMEXEC)
#define permsticky(perm)          ((perm)->flg & PERMSTICKY)
#define permsetgid(perm)          ((perm)->flg & PERMSETGID)
#define permsetuid(perm)          ((perm)->flg & PERMSETUID)
#define permwire(perm)            ((perm)->flg & PERMWIRE)
#define permshmap(perm)           ((perm)->flg & PERMSHMAP)
#define permrawio(perm)           ((perm)->flg & PERMRAWIO)

struct perm {
    long uid;   // user ID
    long gid;   // group ID
    long flg;   // access bits
};

#endif /* __KERN_PERM_H__ */

