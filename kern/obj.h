#ifndef __KERN_OBJ_H__
#define __KERN_OBJ_H__

#include <kern/perm.h>

/* type values */
#define OBJNOTYPE 0
#define OBJCDEV   1
#define OBJBDEV   2
#define OBJFILE   3
#define OBJDIR    4
#define OBJSOCK   5
#define OBJMQ     6
#define OBJSEM    7
#define OBJSHM    8
#define OBJLFB    9
/* flg values */
struct desc {
    long         type;  /* CDEV, BDEV, FILE, DIR, SOCK, MQ, SEM, SHM, LFB */
    long         flg;   /* type-specific flags */
    long         nref;  /* reference count */
    struct perm  perm;  /* permission structure */
    void        *atr;   /* type-specific attributes */
};

#endif /* __KERN_OBJ_H__ */

