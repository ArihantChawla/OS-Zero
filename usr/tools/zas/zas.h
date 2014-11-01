#ifndef __ZAS_ZAS_H__
#define __ZAS_ZAS_H__

#include <stdint.h>

#include <stdint.h>

#if (WPM)
#define ZASNREG     NREG
#define ZASREGINDIR REGINDIR
#define ZASREGINDEX REGINDEX
#endif

#if (ZAS32BIT)
typedef uint32_t  zasmemadr_t;
typedef int32_t   zasword_t;
typedef uint32_t  zasuword_t;
typedef uint32_t  zassize_t;
#else /* 64-bit */
typedef uint64_t  zasmemadr_t;
typedef int64_t   zasword_t;
typedef uint64_t  zasuword_t;
typedef uint64_t  zassize_t;
#endif

#if (WPM)
#include <wpm/wpm.h>
#elif (ZEN)
#include <zpu/zpu.h>
#elif (ZVM)
#include <zvm/zvm.h>
#endif

#define LINELEN     4096

#define TOKENVALUE  0x01
#define TOKENLABEL  0x02
#define TOKENINST   0x03
#define TOKENREG    0x04
#define TOKENVAREG  0x05
#define TOKENVLREG  0x06
#define TOKENSYM    0x07
#define TOKENCHAR   0x08
#define TOKENIMMED  0x09
#define TOKENINDIR  0x0a
#define TOKENADR    0x0b
#define TOKENINDEX  0x0c
#define TOKENDATA   0x0d
#define TOKENGLOBL  0x0e
#define TOKENSPACE  0x0f
#define TOKENORG    0x10
#define TOKENALIGN  0x11
#define TOKENASCIZ  0x12
#define TOKENSTRING 0x13
#define TOKENPAREN  0x14
#if (ZASPREPROC)
#define TOKENOP     0x15
#endif
#define NTOKTYPE    0x16

struct zasopinfo {
    const char       *name;
    uint8_t           narg;
};

struct zasop {
    uint8_t      *name;
    uint8_t       code;
    uint8_t       narg;
    uint8_t       len;
#if (WPMVEC)
    uint32_t      flg;
#endif
    struct zasop *next;
#if (ZVM)
    struct zasop *tab;
#endif
};

struct zaslabel {
    uint8_t         *name;
    uintptr_t        adr;
    struct zaslabel *next;
};

struct zasvalue {
    zasword_t val;
    uint8_t   size;
};

struct zasinst {
    uint8_t *name;
#if (ZASDB)
    uint8_t *data;
#endif
    uint8_t  op;
    uint8_t  narg;
#if (WPMVEC)
    uint8_t  flg;
#endif
};

struct zassymrec {
    struct zassymrec *next;
    uint8_t          *name;
    uintptr_t         adr;
};

struct zassym {
    uint8_t    *name;
    zasuword_t  adr;
};

struct zasadr {
    uint8_t   *name;
    uintptr_t  val;
};

struct zasndx {
    zasword_t reg;
    zasword_t val;
};

struct zasval {
    uint8_t       *name;
    zasword_t      val;
    struct zasval *next;
};

#if (WPMVEC)
#if 0
#define VEC_BYTE  1
#define VEC_WORD  2
#define VEC_LONG  4
#define VEC_QUAD  8
#define VEC_SATU  0x01
#define VEC_SATS  0x02
#endif
#define REG_VA    0x80000000
#define REG_VL    0x40000000
#endif
struct zastoken {
    struct zastoken     *prev;
    struct zastoken     *next;
    unsigned long        type;
    zasword_t            val;
    zasword_t            unit;
#if (WPMVEC)
    uint32_t             opflg;
#endif
#if (ZASDB)
    uint8_t             *file;
    unsigned long        line;
#endif
    union {
        struct zaslabel  label;
        struct zasvalue  value;
        struct zasinst   inst;
        struct zassym    sym;
        struct zasadr    adr;
        struct zasndx    ndx;
        uint8_t         *str;
        uint8_t          ch;
        uint8_t          size;
        zasuword_t       reg;
    } data;
};

#if (ZASDB) || (WPMDB)
struct zasline {
    struct zasline *next;
    uintptr_t       adr;
    uint8_t        *file;
    unsigned long   num;
    uint8_t        *data;
};

struct zasline * zasfindline(zasmemadr_t adr);
#endif

void        zasinit(struct zasopinfo *opinfotab, struct zasopinfo *vecinfotab);
zasmemadr_t zastranslate(zasmemadr_t base);
void        zasresolve(zasmemadr_t base);
void        zasremovesyms(void);
#if (ZASBUF)
void        zasreadfile(char *name, zasmemadr_t adr, int bufid);
#else
void        zasreadfile(char *name, zasmemadr_t adr);
#endif

void        zasfreetoken(struct zastoken *token);
void        zasqueuesym(struct zassymrec *sym);

#endif /* __ZAS_ZAS_H__ */

