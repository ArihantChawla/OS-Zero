#ifndef __KERN_SHM_H__
#define __KERN_SHM_H__

/* shared memory regions are mapped to physical by zero-on-allocation */

/* permission flags */
/* basic unix rwx-flags for user, group, and world */
#define SHM_UX     0x0001       // user can execute
#define SHM_UW     0x0002       // user can write
#define SHM_UR     0x0004       // user can read
#define SHM_GX     0x0008       // group can execute
#define SHM_GW     0x0010       // group can write
#define SHM_GR     0x0020       // group can execute
#define SHM_WX     0x0040       // everyone (incl. anonymous) can execute
#define SHM_WW     0x0100       // everyone can write
#define SHM_WR     0x0200       // everyone can read
/* other permissions */
#define SHM_SINGLE 0x0400       // single user process (pid) can access
struct shmreg {
    pid_t     pid;      // process ID of owner
    uid_t     uid;      // user ID of owner
    uid_t     gid;      // group ID of owner
    long      perm;     // permissions
    uintptr_t adr;      // kernel virtual memory address
    size_t    size;     // size in bytes (octets)
};

#endif /* __KERN_SHM_H__ */

