#ifndef __ZVM_ZVM_H__
#define __ZVM_ZVM_H__

#include <stddef.h>
#include <stdint.h>
#include <zero/param.h>
#include <zero/cdecl.h>
#include <zas/zas.h>

#if (ZVMVIRTMEM)
#define ZASTEXTBASE     ZVMPAGESIZE
#define ZASNPAGE        (1UL << ZVMADRBITS - PAGESIZELOG2)
#if (ZAS32BIT)
#define ZVMADRBITS      32
#else
#define ZVMADRBITS      ADRBITS
#endif
#define ZVMPAGESIZE     PAGESIZE
#define ZVMPAGESIZELOG2 PAGESIZELOG2
#define ZVMPAGEPRES     0x01
#define ZVMPAGEREAD     0x02
#define ZVMPAGEWRITE    0x04
#define ZVMPAGEEXEC     0x08
#else
#define ZASTEXTBASE     PAGESIZE
#define ZVMMEMSIZE      (128U * 1024U * 1024U)
#endif
#define ZASNREG         32

#define ZASREGINDEX     0x04
#define ZASREGINDIR     0x08
#define ZVMARGIMMED     0x00
#define ZVMARGREG       0x01
#define ZVMARGADR       0x02

/* number of units and instructions */
#define ZVMNUNIT        16
#define ZVMNOP          256

/* machine status word */
#define ZVMZF           0x01 // zero
#define ZVMOF           0x02 // overflow
#define ZVMCF           0x04 // carry
#define ZVMIF           0x08 // interrupt pending

/*
 * - unit
 * - operation code (within unit)
 * - flg  - addressing flags (REG, IMMED, INDIR, ...)
 */
struct zvmopcode {
    unsigned int  unit  : 4;
    unsigned int  inst  : 4;
    unsigned int  arg1t : 4;
    unsigned int  arg2t : 4;
    unsigned int  reg1  : 6;
    unsigned int  reg2  : 6;
    unsigned int  size  : 4;
#if (!ZAS32BIT)
    unsigned int  pad   : 32;
#endif
    unsigned long args[EMPTY];
} PACK();

typedef void zvmopfunc_t(struct zvmopcode *);

struct zvm {
    zasword_t    regs[ZASNREG] ALIGNED(CLSIZE);
#if !(ZVMVIRTMEM)
    uint8_t     *physmem;
    size_t       memsize;
#endif
    long         shutdown;
    zasword_t    msw;
    zasword_t    fp;
    zasword_t    sp;
    zasword_t    pc;
#if (ZASVIRTMEM)
    void       **pagetab;
#endif
};

#if (ZVMVIRTMEM)
#define zvmpagenum(adr)                                                 \
    ((adr) >> ZVMPAGESIZELOG2)
#define zvmreadmem(adr, t)                                              \
    (
#else
#define zvmreadmem(adr, t)                                              \
    (*(t *)&zvm.physmem[(adr)])
#endif
#define zvmgetmem(adr)                                                  \
    ((adr) & (sizeof(zasword_t) - 1)                                    \
     ? zvmsigbus(adr, sizeof(zasword_t))                                \
     : zvmreadmem(adr, zasword_t))
#define zvmgetmemt(adr, t)                                              \
    ((adr) & (sizeof(t) - 1)                                            \
     ? zvmsigbus(adr, sizeof(t))                                        \
     : zvmreadmem(adr, t))
#if 0
#define zvmgetmemb(adr)                                                 \
    zvmreadmem(adr, int8_t))
#define zvmgetmemw(adr)                                                 \
    ((adr) & (sizeof(int16_t) - 1)                                      \
     ? zvmsigbus(adr, sizeof(int16_t))                                  \
     : zvmreadmem(adr, int16_t))
#define zvmgetmeml(adr)                                                 \
    ((adr) & (sizeof(int32_t) - 1)                                      \
     ? zvmsigbus(adr, sizeof(int32_t))                                  \
     : zvmreadmem(adr, int32_t))
#define zvmgetmemq(adr)                                                 \
    ((adr) & (sizeof(int64_t) - 1)                                      \
     ? zvmsigbus(adr, sizeof(int64_t))                                  \
     : zvmreadmem(adr, int64_t))
#endif

extern struct zasop  zvminsttab[ZVMNOP];
extern struct zasop *zvmoptab[ZVMNOP];
extern const char   *zvmopnametab[ZVMNOP];
extern const char   *zvmopnargtab[ZVMNOP];
extern struct zvm    zvm;

void              zvminit(void);
size_t            zvminitmem(void);
struct zasop    * asmaddop(const uint8_t *str, struct zasop *op);
struct zasop    * zvmfindasm(const uint8_t *str);
struct zastoken * zasprocinst(struct zastoken *token, zasmemadr_t adr,
                              zasmemadr_t *retadr);

void            * zvmsigbus(zasmemadr_t adr, size_t size);

#endif /* __ZVM_ZVM_H__ */

