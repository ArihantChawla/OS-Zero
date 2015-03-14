#ifndef __ZVM_ZVM_H__
#define __ZVM_ZVM_H__

#include <stddef.h>
#include <stdint.h>
#include <zero/param.h>
#include <zero/cdecl.h>
#include <zas/zas.h>

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

struct zvmopcode {
    unsigned int code  : 8; // (unit << 4) | inst
    unsigned int arg1t : 4; // argument #1 type
    unsigned int arg2t : 4; // argument #2 type
    unsigned int reg1  : 6; // registér #1 + ZASREGINDIR or ZASREGINDEX
    unsigned int reg2  : 6; // registér #2 + ZASREGINDIR or ZASREGINDEX
    unsigned int size  : 4; // instruction size
#if (!ZAS32BIT)
    unsigned int pad   : 32;
#endif
    zasword_t    args[EMPTY]; // optional arguments
} PACK();

typedef void zvmopfunc_t(struct zvmopcode *);

struct zvm {
    zasword_t       regs[ZASNREG]; // virtual registers
#if !(ZVMVIRTMEM)
    uint8_t        *physmem; // memory base address
    size_t          memsize; // memory size in bytes
#endif
    volatile long   shutdown; // shutdown flag
    zasword_t       msw;      // machine status word
    zasword_t       fp;       // frame pointer
    zasword_t       sp;       // stack pointer
    zasword_t       pc;       // program counter ("instruction pointer")
#if (ZASVIRTMEM)
    void          **pagetab;  // virtual memory structure
#endif
} ALIGNED(CLSIZE);

/* external declarations */
struct zasop  zvminsttab[ZVMNOP];
struct zasop *zvmoptab[ZVMNOP];
const char   *zvmopnametab[ZVMNOP];
const char   *zvmopnargtab[ZVMNOP];
struct zvm    zvm;

/* function prototypes */
void              zvminit(void);
void              zvminitopt(void);
size_t            zvminitmem(void);
void              zvminitio(void);
long              asmaddop(const uint8_t *str, struct zasop *op);
struct zasop    * zvmfindasm(const uint8_t *str);
struct zastoken * zasprocinst(struct zastoken *token, zasmemadr_t adr,
                                     zasmemadr_t *retadr);
int8_t            zvmsigbus(zasmemadr_t adr, long size);
int8_t            zvmsigsegv(zasmemadr_t adr, long reason);
void              asmprintop(struct zvmopcode *op);

#endif /* __ZVM_ZVM_H__ */

