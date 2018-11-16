#ifndef __KERN_OBJ_H__
#define __KERN_OBJ_H__

#include <mach/types.h>
#include <kern/perm.h>
#include <kern/acl.h>
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
#define OBJ_HID    10
#define OBJ_LOCK   11
#define OBJ_LFB    12
#define OBJ_EVQ    13

/* the structure we wait on */
/* for sleeping, waketm is wakeup time (system tick ID) */
/* wait-channel is simply the address of this header as a uintptr_t */
struct waitobj {
    m_ureg_t     type;   // object type
    m_atomic_t   nref;   // reference count
    long         wakeup; // tick ID for waking up
    struct task *queue;  // waiter queue
};

/* flg values */
#define IO_PRIO_MASK  0xff       // I/O-priority
#define IO_RAW_BIT    (1U << 8)  // raw/character-based I/O
#define IO_BUF_BIT    (1U << 9)  // buffered I/O
#define IO_SEEK_BIT   (1U << 10) // seekable device
#define IO_RAD_BIT    (1U << 11) // random-access device (no need to seek)
#define IO_DMA_BIT    (1U << 12) // direct-memory transfers
#define IO_MAPPED_BIT (1U << 13) // mapped I/O-object
#define IO_SHARED_BIT (1U << 14) // shared I/O-object
#define IO_FSYNC_BIT  (1U << 15) // filesystem-level synchronization
#define IO_DSYNC_BIT  (1U << 16) // device-level synchronication
struct desc {
    struct waitobj  wait;
    long            flg;        /* type-specific flags */
    struct perm     perm;       /* permission structure */
    void           *obj;        /* type-specific attributes */
    struct acl     *acl;        /* access control list */
};

#endif /* __KERN_OBJ_H__ */

