#ifndef __KERN_OBJ_H__
#define __KERN_OBJ_H__

#include <stdint.h>
#include <signal.h>
#include <sys/types.h>
#include <zero/param.h>
#include <zero/cdecl.h>
#include <zero/types.h>
#include <kern/types.h>
//#include <kern/syscall.h>
#include <kern/proc/thr.h>

/* user + group credentials */
struct cred {
    uid_t uid;                          // user ID
    gid_t gid;                          // group ID
} PACK();

/* permissions */
struct perm {
    struct cred owner;                  // user + group
    long        flg;                    // permission bits
};

/* memory region */
struct memreg {
    struct perm   perm;                 // memory permissions
    unsigned long flg;                  // flags
    uintptr_t     base;                 // base address
    unsigned long size;                 // size in bytes
    off_t         ofs;                  // region offset
};

/* I/O node */
struct node {
    struct perm   perm; 		// node permissions
    unsigned long type; 		// file, dir, pipe, mq, shm, sock
    desc_t        desc; 		// system descriptor
    unsigned long flg;  		// NODESEEKBIT, NODEMAPBIT, etc.
    unsigned long nref; 		// reference count
    off_t         pos;                  // seek position
    /* TODO: interface function pointers */
};

#endif /* __KERN_OBJ_H__ */

