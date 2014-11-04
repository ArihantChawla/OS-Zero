#ifndef __ZVM_ZVM_H__
#define __ZVM_ZVM_H__

#include <stddef.h>
#include <stdint.h>
#include <zero/param.h>
#include <zero/cdecl.h>
#include <zas/zas.h>

#define ZASMEMSIZE  (128U * 1024U * 1024U)
#define ZASNREG     16
#define ZASTEXTBASE PAGESIZE

#define ZASREGINDEX 0x04
#define ZASREGINDIR 0x08
#define ZVMARGIMMED 0x00
#define ZVMARGREG   0x01
#define ZVMARGADR   0x02

/* number of units and instructions */
#define ZVMNUNIT    16
#define ZVMNOP      256

/* machine status word */
#define ZVMZF       0x01 // zero
#define ZVMOF       0x02 // overflow
#define ZVMCF       0x04 // carry
#define ZVMIF       0x08 // interrupt pending

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
    uint8_t   *physmem;
    size_t     memsize;
    long       shutdown;
    zasword_t  msw;
    zasword_t  fp;
    zasword_t  sp;
    zasword_t  pc;
    zasword_t  regs[ZASNREG] ALIGNED(CLSIZE);
};

extern struct zasop  zvminsttab[ZVMNOP];
extern struct zasop *zvmoptab[ZVMNOP];
extern const char   *zvmopnametab[ZVMNOP];
extern const char   *zvmopnargtab[ZVMNOP];
extern struct zvm    zvm;

struct zasop    * zvmfindasm(const uint8_t *str);
struct zastoken * zasprocinst(struct zastoken *token, zasmemadr_t adr,
                              zasmemadr_t *retadr);

#endif /* __ZVM_ZVM_H__ */

