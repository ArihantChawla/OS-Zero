#ifndef __KERN_OBJ_H__
#define __KERN_OBJ_H__

#include <kern/perm.h>
#include <kern/lfb.h>

/* type values */
#define OBJ_NOTYPE 0
#define OBJ_CDEV   1
#define OBJ_BDEV   2
#define OBJ_FILE   3
#define OBJ_DIR    4
#define OBJ_PIPE   5
#define OBJ_SOCK   6
#define OBJ_MQ     7
#define OBJ_SEM    8
#define OBJ_SHM    9
#define OBJ_LFB    10
/* flg values */
#define IO_NOBUF   (1 << 1)
#define IO_CANSEEK (1 << 2)
#define IO_DMA     (1 << 3)
#define IO_MAPPED  (1 << 4)
struct desc {
    long         type;  /* CDEV, BDEV, FILE, DIR, SOCK, MQ, SEM, SHM, LFB */
    long         flg;   /* type-specific flags */
    long         nref;  /* reference count */
    struct perm  perm;  /* permission structure */
    void        *obj;   /* type-specific attributes */
};

#endif /* __KERN_OBJ_H__ */

