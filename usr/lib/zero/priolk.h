#ifndef __ZERO_PRIOLK_H__
#define __ZERO_PRIOLK_H__

/*
 * REFERENCE: http://locklessinc.com/articles/priority_locks/
 */

#define priolkwait()                                                    \
    do {                                                                \
        __asm__ __volatile__ ("pause\n" : : : "memory");                \
    } while (0)

struct priolkdata {
    unsigned long      val;
    unsigned long      orig;
    struct priolkdata *next;
};

struct priolk {
    volatile struct priolkdata *owner;
    volatile unsigned long      waitbits;
};

void priolkinit(struct priolkdata *data, unsigned long val);
void priolk(struct priolk *priolk);
void priounlk(struct priolk *priolk);

#endif /* __ZERO_PRIOLK_H__ */

