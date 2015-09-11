#ifndef __KERN_DC_H__
#define __KERN_DC_H__

struct dcstr {
    uint32_t             hash;
    uint32_t             len;
    const unsigned char *name;
};

struct dcops {
    long (*isvalid)(struct dcent *ent, struct inodedata *);
    long (*hash)(struct dcent *, struct dcstr *);
    long (*cmp)(struct dcent *, struct dcstr *, struct dcstr *); // LOCK
    long (*del)(struct dcent *);
    void (*free)(struct dcent *);
    void (*iput)(struct dcent *, struct inode *);
    char *(*name)(struct dcent *, char *, long);
};

struct dc {
    ;
};

#endif /* __KERN_DC_H__ */

