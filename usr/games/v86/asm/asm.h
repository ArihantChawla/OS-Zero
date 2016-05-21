#ifndef __V86_ASM_H__
#define __V86_ASM_H__

#include <v86asm/op.h>

#if (V86_ASM_OP_HASH_ITEMS <= 65536)
typedef uint16_t v86asmopkey;
#endif

/* assembly symbols */
#define V86_ASM_SYM_MAX_CHARS      15 // not counting the trailing '\0' (NUL)
#define V86_ASM_SYM_CHAR_BITS      6  // A-Z, a-z, 0-9, _ - total of 63 chars
#define V86_ASM_SYM_HASH_ITEMS     (1U << V86_ASM_SYM_HASH_BITS)
/*
 * # of hash table chains is 1 << V86_ASM_SYM_HASH_BITS - there aren't going to
 * be many instruction descriptions hashed, so setting this too high might be
 * overkill and little use. :)
 */
#define V86_ASM_SYM_HASH_BITS      10

/* assembly mnemonics */
#define V86_ASM_MNEMONIC_MAX_LEN   8    // maximum # of characters in mnemonic
#define V86_ASM_MNEMONIC_CHAR_BITS 5    // A-Z
#define v86asmismnemchar(c) isalpha(toupper(c))
#define v86asmmnemcharid(c) (toupper(c) - 'A')

/* special adr-member value */
#define V86_ASM_SYM_UNRESOLVED (~(v86asmadr)0)

struct v86asmsym {
    char   *name;
    size_t  namelen;
    long    adr;
    long    val;
};

struct v86asmhashsym {
    struct v86asmsym     *sym;
    struct v86asmhashsym *next;
};

#endif /* __V86_ASM_H__ */

