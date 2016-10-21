#ifndef __KERN_CRED_H__
#define __KERN_CRED_H__

#include <stddef.h>
#include <sys/types.h>

#define NOCRED NULL                     // no cred available
#define FSCRED ((struct ucred *)-1)     // filesystem credential

/* flg-bits */
#define CRED_CAPMODE_BIT 0x00000001
#define ucred_startcopy(cp) (&(cp)->uid)
#define ucred_endcopy(cp)   (&(cp)->grptab)
struct ucred {
    volatile long  nref;
    long           flg;
    uid_t          uid;         // effective user ID
    uid_t          ruid;        // real user ID
    uid_t          suid;        // saved user ID
    gid_t          rgid;        // real group ID
    gid_t          sgid;        // saved group ID
    uid_t          fsuid;       // for VFS operations
    gid_t          fsgid;       // for VFS operations
    long           ngrp;        // # of groups
    gid_t         *grptab;      // group IDs
};
#define credgid(uc) ((uc)->grptab[0])

#endif /* __KERN_CRED_H__ */

