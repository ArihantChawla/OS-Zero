#ifndef __CPUNK_MACHINE_H__
#define __CPUNK_MACHINE_H__

#define CPUNK_MACHINE_MOBILE    0
#define CPUNK_MACHINE_LAPTOP    1
#define CPUNK_MACHINE_DESKTOP   2
#define CPUNK_MACHINE_MAINFRAME 3
#define CPUNK_MACHINE_SUPER     4
struct cpunkmachine {
    long     cpu;
    long     fpu;
    long     nbmem;
    uint8_t *mem;
    long     nbtext;
    uint8_t *text;
    long     nbdata;
    uint8_t *data;
    long     nbbss;
    uint8_t *bss;
    long     nbstk;
    uint8_t *stk;
    uint8_t *stklim;
};

#define CPUNK_MACHINE_NBSTK  4096
#define machpush(mp, val)                                               \
    (((mp)->stk == (mp)->stklim)                                        \
     ? abort()                                                          \
     : (*(--(mp)->stk) = (val)))
#define machpop(mp, nb)                                                 \
    (((mp->stk == stklim + CPUNK_MACHINE_NBSTK - nb)                    \
    ? abort()                                                           \
    : ((val) = *((mp)->stk++))))

#endif /* __CPUNK_MACHINE_H__ */

