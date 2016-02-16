#ifndef __KERN_ACL_H__
#define __KERN_ACL_H__

#include <zero/param.h>
#include <zero/trix.h>
#include <kern/types.h>

/* FIXME: assert size/alignments! */
#define __STRUCT_ACL_SIZE                                               \
    (sizeof(long) + 4 * sizeof(void *)                                  \
     + sizeof(size_t) + sizeof(uid_t) + sizeof(gid_t))
#define __STRUCT_ACL_PAD                                                \
    (roundup(__STRUCT_ACL_SIZE, CLSIZE) - __STRUCT_ACL_SIZE)
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
    uint8_t      _pad[__STRUCT_ACL_PAD];
};

#endif /* __KERN_ACL_H__ */

