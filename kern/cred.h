#ifndef __KERN_CRED_H__
#define __KERN_CRED_H__

#include <sys/types.h>

struct cred {
    volatile long nref;
    volatile long nproc;
    pid_t         pid;          // process ID
    uid_t         ruid;         // real user ID
    gid_t         rgid;         // real group ID
    uid_t         suid;         // saved user ID
    uid_t         sgid;         // saved group ID
    uid_t         euid;         // effective user ID
    gid_t         egid;         // effective group ID
    uid_t         fsuid;        // for VFS operations
    gid_t         fsgid;        // for VFS operations
};

#endif /* __KERN_CRED_H__ */

