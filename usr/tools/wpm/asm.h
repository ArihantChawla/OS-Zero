#ifndef __WPM_ASM_H__
#define __WPM_ASM_H__

#include <stdint.h>

#if (ZPC)
typedef uint64_t asmadr_t;
typedef int64_t  asmword_t;
typedef uint64_t asmuword_t;
#elif (WPM)
typedef uint32_t asmadr_t;
typedef int32_t  asmword_t;
typedef uint32_t asmuword_t;
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
#if (ASMPREPROC)
#define TOKENPAREN  0x11
#define TOKENOP     0x12
#endif
#define NTOK        19

#define OPINVAL    0x00
#define RESOLVE    0xffffffffU

struct op {
    uint8_t   *name;
    struct op *next;
    uint8_t    code;
    uint8_t    narg;
    uint8_t    len;
};

struct label {
    uint8_t      *name;
    asmadr_t      adr;
    struct label *next;
};

struct value {
    asmword_t val;
    uint8_t   size;
};

struct inst {
    uint8_t *name;
#if (WPMDB)
    uint8_t *data;
#endif
    uint8_t  op;
    uint8_t  narg;
};

struct sym {
    uint8_t  *name;
    asmadr_t  adr;
};

struct adr {
    uint8_t  *name;
    asmadr_t  val;
};

struct ndx {
    asmword_t reg;
    asmword_t val;
};

struct val {
    uint8_t    *name;
    struct val *next;
    asmword_t   val;
};

struct asmtoken {
    struct asmtoken  *prev;
    struct asmtoken  *next;
    unsigned long     type;
    asmword_t         val;
#if (WPMDB)
    uint8_t          *file;
    unsigned long     line;
#endif
    union {
        struct label  label;
        struct value  value;
        struct inst   inst;
        struct sym    sym;
        struct adr    adr;
        struct ndx    ndx;
        uint8_t      *str;
        uint8_t       ch;
        uint8_t       size;
        asmuword_t    reg;
    } data;
};

#if (WPMDB)
struct asmline {
    struct asmline *next;
    asmadr_t        adr;
    uint8_t        *file;
    unsigned long   num;
    uint8_t        *data;
};

struct asmline * asmfindline(asmadr_t adr);
#endif

#endif /* __WPM_ASM_H__ */

