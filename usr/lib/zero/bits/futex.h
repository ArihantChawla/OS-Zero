#ifndef __ZERO_BITS_FUTEX_H__
#define __ZERO_BITS_FUTEX_H__

/* the first five operations are documented in the manual page */
#define FUTEX_WAIT                    0
#define FUTEX_WAKE                    1
#define FUTEX_FD                      2
#define FUTEX_REQUEUE                 3
#define FUTEX_CMP_REQUEUE             4
/* the rest of the operations are somewhat-undocumented */
#define FUTEX_WAKE_OP                 5
#define FUTEX_LOCK_PI                 6
#define FUTEX_UNLOCK_PI               7
#define FUTEX_TRYLOCK_PI              8
#define FUTEX_WAIT_BITSET             9
#define FUTEX_WAKE_BITSET             10
#define FUTEX_WAIT_REQUEUE_PI         11
#define FUTEX_CMP_REQUEUE_PI          12
#define FUTEX_PRIVATE_FLAG            0x80000000
#define FUTEX_CLOCK_REALTIME          0x40000000
#define FUTEX_CMD_MASK                (~(FUTEX_PRIVATE_FLAG             \
                                         | FUTEX_CLOCK_REALTIME))
#define FUTEX_WAIT_PRIVATE            (FUTEX_WAIT | FUTEX_PRIVATE_FLAG)
#define FUTEX_WAKE_PRIVATE            (FUTEX_WAKE | FUTEX_PRIVATE_FLAG)
#define FUTEX_REQUEUE_PRIVATE         (FUTEX_REQUEUE | FUTEX_PRIVATE_FLAG)
#define FUTEX_CMP_REQUEUE_PRIVATE     (FUTEX_CMP_REQUEUE | FUTEX_PRIVATE_FLAG)
#define FUTEX_WAKE_OP_PRIVATE         (FUTEX_WAKE_OP | FUTEX_PRIVATE_FLAG)
#define FUTEX_LOCK_PI_PRIVATE         (FUTEX_LOCK_PI | FUTEX_PRIVATE_FLAG) 
#define FUTEX_UNLOCK_PI_PRIVATE       (FUTEX_UNLOCK_PI | FUTEX_PRIVATE_FLAG)
#define FUTEX_TRYLOCK_PI_PRIVATE      (FUTEX_TRYLOCK_PI | FUTEX_PRIVATE_FLAG)
#define FUTEX_WAIT_BITSET_PRIVATE     (FUTEX_WAIT_BITSET | FUTEX_PRIVATE_FLAG)
#define FUTEX_WAKE_BITSET_PRIVATE     (FUTEX_WAKE_BITSET | FUTEX_PRIVATE_FLAG)
#define FUTEX_WAIT_REQUEUE_PI_PRIVATE (FUTEX_WAIT_REQUEUE_PI            \
                                       | FUTEX_PRIVATE_FLAG)       
#define FUTEX_CMP_REQUEUE_PI_PRIVATE  (FUTEX_CMP_REQUEUE_PI \
                                       | FUTEX_PRIVATE_FLAG)
    
                                         struct _listfutex {
    struct _listfutex *next;
};

struct _futexlisthead {
    struct _listfutex list; /* list head, points to itself for empty list */
    long              ofs;  /* relative offset into data structure */
    struct _listfutex pend; /* hint to kernel about pending operation */
};

#endif /* __ZERO_BITS_FUTEX_H__ */

