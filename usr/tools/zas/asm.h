#ifndef __ZAS_ASM_H__
#define __ZAS_ASM_H__

#include <stdint.h>

#if (ZPC)
typedef uint32_t zasmemadr_t;
typedef int64_t  zasword_t;
typedef uint64_t zasuword_t;
#elif (WPM)
typedef uint32_t zasmemadr_t;
typedef int32_t  zasword_t;
typedef uint32_t zasuword_t;
#endif

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
#define NTOK        19

#define OPINVAL    0x00
//#if (ZPC)
//#define RESOLVE    INT64_C(0xffffffffffffffff)
//#elif (WPM)
#define RESOLVE    0xffffffff
//#endif

#define REGINDEX   0x10
#define REGINDIR   0x20
/* argument types */
#define ARGNONE    0x00	// no argument
#define ARGIMMED   0x01	// immediate argument
#define ARGADR     0x02	// symbol / memory address
#define ARGREG     0x03	// register
#define ARGSYM     0x04	// symbol address

struct op {
    uint8_t   *name;
    uint8_t    code;
    uint8_t    narg;
    uint8_t    len;
    struct op *next;
};

struct label {
    uint8_t      *name;
    zasmemadr_t   adr;
    struct label *next;
};

struct value {
    zasword_t val;
    uint8_t   size;
};

struct inst {
    uint8_t *name;
#if (ZASDB)
    uint8_t *data;
#endif
    uint8_t  op;
    uint8_t  narg;
};

struct sym {
    uint8_t     *name;
    zasmemadr_t  adr;
};

struct adr {
    uint8_t     *name;
    zasmemadr_t  val;
};

struct ndx {
    zasword_t reg;
    zasword_t val;
};

struct val {
    uint8_t    *name;
    zasword_t   val;
    struct val *next;
};

struct zastoken {
    struct zastoken     *prev;
    struct zastoken     *next;
    unsigned long        type;
    zasword_t            val;
#if (ZASDB)
    uint8_t             *file;
    unsigned long        line;
#endif
    union {
        struct label     label;
#if (ZPC)
        struct zpctoken *token;
#elif (WPM)
        struct value     value;
#endif
        struct inst      inst;
        struct sym       sym;
        struct adr       adr;
        struct ndx       ndx;
        uint8_t         *str;
        uint8_t          ch;
        uint8_t          size;
        zasuword_t       reg;
    } data;
};

#if (ZASDB)
struct zasline {
    struct zasline *next;
    zasmemadr_t     adr;
    uint8_t        *file;
    unsigned long   num;
    uint8_t        *data;
};

struct zasline * zasfindline(zasmemadr_t adr);
#endif

#endif /* __ZAS_ASM_H__ */

