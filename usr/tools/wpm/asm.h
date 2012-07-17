#ifndef __WPM_ASM_H__
#define __WPM_ASM_H__

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
#define NTOK       17

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
    uint32_t      adr;
    struct label *next;
};

struct value {
    int32_t  val;
    uint8_t  size;
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
    uint32_t  adr;
};

struct adr {
    uint8_t  *name;
    uint32_t  val;
};

struct ndx {
    int32_t reg;
    int32_t val;
};

struct val {
    uint8_t    *name;
    struct val *next;
    int32_t     val;
};

struct token {
    struct token *prev;
    struct token *next;
    uint32_t      type;
    int32_t       val;
#if (WPMDB)
    uint8_t      *file;
    uint32_t      line;
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
        uint32_t      reg;
    } data;
};

#if (WPMDB)
struct asmline {
    struct asmline *next;
    uint32_t        adr;
    uint8_t        *file;
    uint32_t        num;
    uint8_t        *data;
};

struct asmline * asmfindline(uint32_t adr);
#endif

#endif /* __WPM_ASM_H__ */

