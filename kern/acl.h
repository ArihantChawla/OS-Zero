#ifndef __KERN_ACL_H__
#define __KERN_ACL_H__

struct acl {
    long         uid;           // owner user-ID
    long         gid;           // owner group-ID
    long         flg;           // Unix-style permission flags
    long         datalen;       // size of data in bytes
    // permission check routine pointer
    // - uid, gid, flg arguments
    long       (*chk)(long, long, long);
    void        *data;          // permission object data
    struct acl  *prev;          // previous in queue
    struct acl  *next;          // next in queue
};

#endif /* __KERN_ACL_H__ */

