#ifndef __KERN_OBJ_H__
#define __KERN_OBJ_H__

#include <stdint.h>
#include <signal.h>
#include <sys/types.h>
#include <kern/types.h>
#include <kern/perm.h>

/* File-Like Object Interface */

/* object region */
struct objreg {
    struct perm   perm;                 // object permissions
    off_t         ofs;                  // region offset
    uintptr_t     base;                 // base address
    unsigned long size;                 // size in bytes
    unsigned long flg;                  // flags
};

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

