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
#define PERMSETGID 0x00000800
#define PERMSETUID 0x00001000

/* TODO */
#define permcanread(uid, gid, perm)
#define permcanwrite(uid, gid, perm)
#define permcanexec(uid, gid, perm)
#define permsticky(perm)
#define permsetgid(perm)
#define permsetuid(perm)

struct perm {
    long uid;   // user ID
    long gid;   // group ID
    long mask;  // access bitmask
};

#endif /* __KERN_PERM_H__ */

