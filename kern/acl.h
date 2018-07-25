#ifndef __KERN_ACL_H__
#define __KERN_ACL_H__

#include <mach/param.h>
#include <zero/trix.h>
#include <kern/types.h>

/* FIXME: assert size/alignments! */
struct acl {
    uid_t        uid;           // owner user-ID
    gid_t        gid;           // owner group-ID
    long         flg;           // Unix-style permission flags + other bits
    size_t       datalen;       // size of data in bytes
    // permission check routine pointer
    // - uid, gid, flg arguments
    long       (*chk)(long, long, long);
    void        *data;          // permission object data
    struct acl  *prev;          // previous in queue
    struct acl  *next;          // next in queue
};

#endif /* __KERN_ACL_H__ */

