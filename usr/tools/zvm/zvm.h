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
#else /* !ZVMVIRTMEM */
#define ZASTEXTBASE     PAGESIZE
#define ZVMMEMSIZE      (128U * 1024U * 1024U)
#endif
#define ZASNREG         16

#define ZASREGINDEX     0x20
#define ZASREGINDIR     0x10
#define ZVMARGIMMED     0x00
#define ZVMARGREG       0x01
#define ZVMARGADR       0x02
#define ZASREGMASK      0x0f

/* number of units and instructions */
#define ZVMNUNIT        16
#define ZVMNOP          256

/* machine status word */
#define ZVMZF           0x01 // zero
#define ZVMOF           0x02 // overflow
#define ZVMCF           0x04 // carry
#define ZVMIF           0x08 // interrupt pending

/* memory operation failure codes */
#define ZVMMEMREAD      0
#define ZVMMEMWRITE     1

/*
 * - unit
 * - operation code (within unit)
 * - flg  - addressing flags (REG, IMMED, INDIR, ...)
 */
struct zvmopcode {
    unsigned int code  : 8;
//    unsigned int  unit  : 4;
//    unsigned int  inst  : 4;
    unsigned int arg1t : 4;
    unsigned int arg2t : 4;
    unsigned int reg1  : 6;
    unsigned int reg2  : 6;
    unsigned int size  : 4;
#if (!ZAS32BIT)
    unsigned int pad   : 32;
#endif
    zasword_t    args[EMPTY];
} PACK();

typedef void zvmopfunc_t(struct zvmopcode *);

struct zvm {
    zasword_t       regs[ZASNREG] ALIGNED(CLSIZE);
#if !(ZVMVIRTMEM)
    uint8_t        *physmem;
    size_t          memsize;
#endif
    volatile long   shutdown;
    zasword_t       msw;
    zasword_t       fp;
    zasword_t       sp;
    zasword_t       pc;
#if (ZASVIRTMEM)
    void          **pagetab;
#endif
};

/* external declarations */
extern struct zasop  zvminsttab[ZVMNOP];
extern struct zasop *zvmoptab[ZVMNOP];
extern const char   *zvmopnametab[ZVMNOP];
extern const char   *zvmopnargtab[ZVMNOP];
extern struct zvm    zvm;

/* function prototypes */
extern void              zvminit(void);
extern size_t            zvminitmem(void);
extern long              asmaddop(const uint8_t *str, struct zasop *op);
extern struct zasop    * zvmfindasm(const uint8_t *str);
extern struct zastoken * zasprocinst(struct zastoken *token, zasmemadr_t adr,
                                     zasmemadr_t *retadr);
extern int8_t            zvmsigbus(zasmemadr_t adr, long size);
extern int8_t            zvmsigsegv(zasmemadr_t adr, long reason);
extern void              asmprintop(struct zvmopcode *op);

/* memory fetch and store macros */
#define zvmgetmemt(adr, t)                                              \
    (((adr) & (sizeof(t) - 1))                                          \
     ? zvmsigbus(adr, sizeof(t))                                        \
     : _zvmreadmem(adr, t))
#define zvmputmemt(adr, t, val)                                         \
    (((adr) & (sizeof(t) - 1))                                          \
     ? zvmsigbus(adr, sizeof(t))                                        \
     : _zvmwritemem(adr, t, val))
/* memory read and write macros */
#if (ZVMVIRTMEM)
#define zvmadrtoptr(adr)                                                \
    (((!zvm.pagetab[zvmpagenum(ZVMTEXTBASE + adr)])                     \
      ? NULL                                                            \
      : &zvm.pagetab[zvmpagenum(ZVMTEXTBASE + adr)][zvmpageofs(adr)]))
#define _zvmpagenum(adr)                                                \
    ((adr) >> ZVMPAGESIZELOG2)
#define _zvmpageofs(adr)                                                \
    ((adr) & (ZVMPAGESIZE - 1))
#define _zvmreadmem(adr, t)                                             \
    (!(zvm.pagetab[zvmpagenum(adr)])                                    \
     ? zvmsigsegv(adr, ZVMMEMREAD)                                      \
     : (((zvmpageofs(adr) & (sizeof(t) - 1))                            \
         ? zvmsigbus(adr, t)                                            \
         : *(t *)zvm.pagetab[zvmpagenum(adr)][zvmpageofs(t)])))
#define _zvmwritemem(adr, t, val)                                       \
    (!(zvm.pagetab[zvmpagenum(adr)])                                    \
    ? zvmsigsegv(adr, ZVMMEMWRITE)                                      \
    : (((zvmpageofs(adr) & (sizeof(t) - 1))                             \
        ? zvmsigbus(adr, t)                                             \
        : *(t *)(zvm.pagetab[zvmpagenum(adr)][zvmpageofs(t)]) = (val)))
#else /* !ZVMVIRTMEM */
#define zvmadrtoptr(adr)                                                \
    (&zvm.physmem[(adr)])
#define _zvmreadmem(adr, t)                                             \
    (((adr) >= ZVMMEMSIZE)                                              \
     ? zvmsigsegv(adr, ZVMMEMREAD)                                      \
     : *(t *)&zvm.physmem[(adr)])
#define _zvmwritemem(adr, t, val)                                       \
    (((adr) >= ZVMMEMSIZE)                                              \
     ? zvmsigsegv(adr, ZVMMEMWRITE)                                     \
     : *(t *)&zvm.physmem[(adr)] = (val))
#endif

#endif /* __ZVM_ZVM_H__ */

