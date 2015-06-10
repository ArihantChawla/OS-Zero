#ifndef __KERN_PERM_H__
#define __KERN_PERM_H__

#define PERM_OX     0x00000001
#define PERM_OW     0x00000002
#define PERM_OR     0x00000004
#define PERM_GX     0x00000008
#define PERM_GW     0x00000010
#define PERM_GR     0x00000020
#define PERM_UX     0x00000040
#define PERM_UW     0x00000080
#define PERM_UR     0x00000100
#define PERM_VTX    0x00000400
#define PERM_SETGID 0x00000800
#define PERM_SETUID 0x00001000
#define PERMRAWIO  0x00002000   // permit to do raw device I/O operations

#define setperm(perm, usr, grp, flg)                                    \
    do {                                                                \
        (perm)->uid = (usr);                                            \
        (perm)->gid = (grp);                                            \
        (perm)->flg = (flg);                                            \
    } while (0)
/* TODO */
#if 0 /* FIXME */
#define permread(usr, grp, perm)                                        \
    (!(usr)                                                             \
		? 1 \
		: (((usr) == (perm)->uid && ((perm)->mask & PERMUREAD))            \
     || ((grp) == (perm)->gid && ((perm)->mask & PERMGREAD))            \
     || ((perm)->mask & PERMWREAD))
#endif /* 0 */
#define permwrite(usr, grp, perm)
#define permexec(usr, grp, perm)
#define permsticky(perm) ((perm)->mask & PERMSTICKY)
#define permsetgid(perm) ((perm)->mask & PERMSETGID)
#define permsetuid(perm) ((perm)->mask & PERMSETUID)
#define permrawio(perm)  ((perm)->mask & PERMRAWIO)

struct perm {
    long uid;   // user ID
    long gid;   // group ID
    long flg;   // access bits
};

#endif /* __KERN_PERM_H__ */

