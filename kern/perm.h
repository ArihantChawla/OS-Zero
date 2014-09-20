#ifndef __KERN_PERM_H__
#define __KERN_PERM_H__

#define PERMWEXEC  0x00000001
#define PERMWWRITE 0x00000002
#define PERMWREAD  0x00000004
#define PERMGEXEC  0x00000008
#define PERMGWRITE 0x00000010
#define PERMGREAD  0x00000020
#define PERMUEXEC  0x00000040
#define PERMUWRITE 0x00000080
#define PERMUREAD  0x00000100
#define PERMSTICKY 0x00000400
#define PERMVTX    PERMSTICKY
#define PERMSETGID 0x00000800
#define PERMSETUID 0x00001000
#define PERMRAWIO  0x00002000   // permit to do raw device I/O operations

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
    long mask;  // access bitmask
};

#endif /* __KERN_PERM_H__ */

