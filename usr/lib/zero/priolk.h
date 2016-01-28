#ifndef __ZERO_PRIOLK_H__
#define __ZERO_PRIOLK_H__

/*
 * REFERENCE: http://locklessinc.com/articles/priority_locks/
 */

#define priolkyield()                                                   \
    do {                                                                \
        __asm__ __volatile__ ("pause\n" : : : "memory");                \
    } while (0)
#define priolkcmpswap(ptr, want, val)                                   \
    ((volatile struct priolkdata *)m_cmpswapptr((volatile long *)(ptr), \
                                                (volatile long *)(want), \
                                                (volatile long *)(val)))

struct priolkdata {
    unsigned long      val;
    unsigned long      orig;
    struct priolkdata *next;
};

struct priolk {
    volatile struct priolk *owner;
    volatile unsigned long  waitbits;
};

#endif /* __ZERO_PRIOLK_H__ */

