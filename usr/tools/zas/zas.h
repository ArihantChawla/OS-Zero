#ifndef __ZAS_ZAS_H__
#define __ZAS_ZAS_H__

#if defined(ZAS32BIT)
typedef uint32_t  zasmemadr_t;
typedef int32_t   zasword_t;
typedef uint32_t  zasuword_t;
typedef uint32_t  zassize_t;
#endif

#include <stdint.h>

#define LINELEN    1024

#define TOKENVALUE  0x01
#define TOKENLABEL  0x02
#define TOKENINST   0x03
#define TOKENREG    0x04
#define TOKENSYM    0x05
#define TOKENCHAR   0x06
#define TOKENIMMED  0x07
#define TOKENINDIR  0x08
#define TOKENADR    0x09
#define TOKENINDEX  0x0a
#define TOKENDATA   0x0b
#define TOKENGLOBL  0x0c
#define TOKENSPACE  0x0d
#define TOKENORG    0x0e
#define TOKENALIGN  0x0f
#define TOKENASCIZ  0x10
#define TOKENSTRING 0x10
#if (ZASPREPROC)
#define TOKENPAREN  0x11
#define TOKENOP     0x12
#endif
#define NTOKTYPE    19

struct zasopinfo {
    const char *name;
    uint8_t     narg;
};

struct zasop {
    uint8_t   *name;
    uint8_t    code;
    uint8_t    narg;
    uint8_t    len;
    struct zasop *next;
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
};

struct zassym {
    uint8_t   *name;
    uintptr_t  adr;
};

struct zasadr {
    uint8_t     *name;
    zasmemadr_t  val;
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

struct zastoken {
    struct zastoken     *prev;
    struct zastoken     *next;
    unsigned long        type;
    zasword_t            val;
#if (ZPC)
    struct zpctoken     *token;
#endif
#if (ZASDB)
    uint8_t             *file;
    unsigned long        line;
#endif
    union {
        struct zaslabel  label;
#if (!ZPC)
        struct zasvalue  value;
#endif
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

#if (ZASDB)
struct zasline {
    struct zasline *next;
    uintptr_t       adr;
    uint8_t        *file;
    unsigned long   num;
    uint8_t        *data;
};

struct zasline * zasfindline(zasmemadr_t adr);
#endif

void        zasinit(struct zasopinfo *opinfo);
zasmemadr_t zastranslate(zasmemadr_t base);
void        zasresolve(zasmemadr_t base);
void        zasremovesyms(void);
#if (ZASBUF)
void        zasreadfile(char *name, zasmemadr_t adr, int bufid);
#else
void        zasreadfile(char *name, zasmemadr_t adr);
#endif

#endif /* __ZAS_ZAS_H__ */

