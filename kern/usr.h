#ifndef __KERN_USR_H__
#define __KERN_USR_H__

#include <sys/types.h>

struct usr {
    const char *name;   // user-name
    const char *shell;  // program executable name
    char       *path;   // user's effective path-string
    /* user credentials */
    uid_t       ruid;   // real user ID
    gid_t       rgid;   // real group ID
    uid_t       suid;   // saved user ID
    uid_t       sgid;   // saved group ID
    uid_t       euid;   // effective user ID
    gid_t       egid;   // effective group ID
    uid_t       fsuid;  // for VFS operations
    gid_t       fsgid;  // for VFS operations
};

#endif /* __KERN_USR_H__ */

