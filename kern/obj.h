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

#if 0

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

#endif

/* File-Like Object Interface */

/* object region */
struct objreg {
    struct perm   perm;                 // memory permissions
    off_t         ofs;                  // region offset
    uintptr_t     base;                 // base address
    unsigned long size;                 // size in bytes
    unsigned long flg;                  // flags
};

#if 0
struct objreg {
    struct perm perm;
    long        desc;
    long        ofs;
    long        len;
};

/* memory region */
struct memreg {
    struct perm   perm;                 // memory permissions
    uintptr_t     base;                 // base address
    unsigned long size;                 // size in bytes
    unsigned long flg;                  // flags
    off_t         ofs;                  // region offset
};
#endif /* 0 */

struct semq {
	struct semthr *head;
	struct semthr *tail;
};

struct semthr {
	long        id;
	struct sem *next;
};

struct sem {
	long        lk; // access lock
	long        val; // counter value
	struct semq q; // queue of waiting threads
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

