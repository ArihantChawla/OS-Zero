#include <ctype.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <zcc/zcc.h>
#define HASHDEBUG 1
#if (HASHDEBUG)
#include <stdio.h>
#endif

#define NSYMHASH   65536
#define NTYPEHASH  65536
#if (NEWHASH)
#define NSTRHASH   65536
#else
#define NSTRTABLVL 64
#endif

static struct zccsym   *symhash[NSYMHASH];
static struct zcctoken *typehash[NTYPEHASH];

#if (NEWHASH)
struct hashstr *qualhash[NSTRHASH];
struct hashstr *preprochash[NSTRHASH];
struct hashstr *atrhash[NSTRHASH];
#else
struct hashstr qualhash[NSTRTABLVL];
struct hashstr preprochash[NSTRTABLVL];
struct hashstr atrhash[NSTRTABLVL];
#endif

/*
 * ISO 8859-1 value compression table
 * Characters for identifier names have codes 0x00..0x3f
 * Other characters are [currently] marked invalid
 */
#define CHVALINVAL 0xffff
static uint16_t chvaltab[256] =
{
    /* ASCII values */

    /* 0x00..0x07 */
    CHVALINVAL, // NUL
    CHVALINVAL, // SOH
    CHVALINVAL, // STX
    CHVALINVAL, // ETX
    CHVALINVAL, // EOT
    CHVALINVAL, // ENQ
    CHVALINVAL, // ACK
    CHVALINVAL, // BEL
    /* 0x08..0x0f */
    CHVALINVAL, // BS
    CHVALINVAL, // HT
    CHVALINVAL, // LF
    CHVALINVAL, // VT
    CHVALINVAL, // FF
    CHVALINVAL, // CR
    CHVALINVAL, // SO
    CHVALINVAL, // SI
    /* 0x10..0x17 */
    CHVALINVAL, // DLE
    CHVALINVAL, // DC1
    CHVALINVAL, // DC2
    CHVALINVAL, // DC3
    CHVALINVAL, // DC4
    CHVALINVAL, // NAK
    CHVALINVAL, // SYN
    CHVALINVAL, // ETB
    /* 0x18..0x1f */
    CHVALINVAL, // CAN
    CHVALINVAL, // EM
    CHVALINVAL, // SUB
    CHVALINVAL, // ESC
    CHVALINVAL, // FS
    CHVALINVAL, // GS
    CHVALINVAL, // RS
    CHVALINVAL, // US
    /* 0x20..0x27 */
    CHVALINVAL, // SPACE
    CHVALINVAL, // !
    CHVALINVAL, // "
    CHVALINVAL, // #
    CHVALINVAL, // $
    CHVALINVAL, // %
    CHVALINVAL, // &
    CHVALINVAL, // '
    /* 0x28..0x2f */
    CHVALINVAL, // (
    CHVALINVAL, // )
    CHVALINVAL, // *
    CHVALINVAL, // +
    CHVALINVAL, // ,
    CHVALINVAL, // -
    CHVALINVAL, // .
    CHVALINVAL, // /
    /* 0x30..0x37 */
    0x00,       // 0
    0x01,       // 1
    0x02,       // 2
    0x03,       // 3
    0x04,       // 4
    0x05,       // 5
    0x06,       // 6
    0x07,       // 7
    /* 0x38..0x3f */
    0x08,       // 8
    0x09,       // 9
    CHVALINVAL, // :
    CHVALINVAL, // ;
    CHVALINVAL, // <
    CHVALINVAL, // =
    CHVALINVAL, // >
    CHVALINVAL, // ?
    /* 0x40..0x47 */
    0x0a,       // @
    0x0b,       // A
    0x0c,       // B
    0x0d,       // C
    0x0e,       // D
    0x0f,       // E
    0x10,       // F
    0x11,       // G
    /* 0x48..0x4f */
    0x12,       // H
    0x13,       // I
    0x14,       // J
    0x15,       // K
    0x16,       // L
    0x17,       // M
    0x18,       // N
    0x19,       // O
    /* 0x50..0x57 */
    0x1a,       // P
    0x1b,       // Q
    0x1c,       // R
    0x1d,       // S
    0x1e,       // T
    0x1f,       // U
    0x20,       // V
    0x21,       // W
    /* 0x58..0x5f */
    0x22,       // X
    0x23,       // Y
    0x24,       // Z
    CHVALINVAL, // [
    CHVALINVAL, // '\'
    CHVALINVAL, // ]
    CHVALINVAL, // ^
    0x25,       // _
    /* 0x60..0x67 */
    CHVALINVAL, // `
    0x26,       // a
    0x27,       // b
    0x28,       // c
    0x29,       // d
    0x2a,       // e
    0x2b,       // f
    0x2c,       // g
    /* 0x68..0x6f */
    0x2d,       // h
    0x2e,       // i
    0x2f,       // j
    0x30,       // k
    0x31,       // l
    0x32,       // m
    0x33,       // n
    0x34,       // o
    /* 0x70..0x77 */
    0x35,       // p
    0x36,       // q
    0x37,       // r
    0x38,       // s
    0x39,       // t
    0x3a,       // u
    0x3b,       // v
    0x3c,       // w
    /* 0x78..0x7f */
    0x3d,       // x
    0x3e,       // y
    0x3f,       // z
    CHVALINVAL, // {
    CHVALINVAL, // |
    CHVALINVAL, // }
    CHVALINVAL, // ~
    CHVALINVAL, // DEL

    /* high 8-bit half (invalid in identifier names) */

    CHVALINVAL,
    CHVALINVAL,
    CHVALINVAL,
    CHVALINVAL,
    CHVALINVAL,
    CHVALINVAL,
    CHVALINVAL,
    CHVALINVAL,

    CHVALINVAL,
    CHVALINVAL,
    CHVALINVAL,
    CHVALINVAL,
    CHVALINVAL,
    CHVALINVAL,
    CHVALINVAL,
    CHVALINVAL,

    CHVALINVAL,
    CHVALINVAL,
    CHVALINVAL,
    CHVALINVAL,
    CHVALINVAL,
    CHVALINVAL,
    CHVALINVAL,
    CHVALINVAL,

    CHVALINVAL,
    CHVALINVAL,
    CHVALINVAL,
    CHVALINVAL,
    CHVALINVAL,
    CHVALINVAL,
    CHVALINVAL,
    CHVALINVAL,

    CHVALINVAL,
    CHVALINVAL,
    CHVALINVAL,
    CHVALINVAL,
    CHVALINVAL,
    CHVALINVAL,
    CHVALINVAL,
    CHVALINVAL,

    CHVALINVAL,
    CHVALINVAL,
    CHVALINVAL,
    CHVALINVAL,
    CHVALINVAL,
    CHVALINVAL,
    CHVALINVAL,
    CHVALINVAL,

    CHVALINVAL,
    CHVALINVAL,
    CHVALINVAL,
    CHVALINVAL,
    CHVALINVAL,
    CHVALINVAL,
    CHVALINVAL,
    CHVALINVAL,

    CHVALINVAL,
    CHVALINVAL,
    CHVALINVAL,
    CHVALINVAL,
    CHVALINVAL,
    CHVALINVAL,
    CHVALINVAL,
    CHVALINVAL,

    CHVALINVAL,
    CHVALINVAL,
    CHVALINVAL,
    CHVALINVAL,
    CHVALINVAL,
    CHVALINVAL,
    CHVALINVAL,
    CHVALINVAL,

    CHVALINVAL,
    CHVALINVAL,
    CHVALINVAL,
    CHVALINVAL,
    CHVALINVAL,
    CHVALINVAL,
    CHVALINVAL,
    CHVALINVAL,

    CHVALINVAL,
    CHVALINVAL,
    CHVALINVAL,
    CHVALINVAL,
    CHVALINVAL,
    CHVALINVAL,
    CHVALINVAL,
    CHVALINVAL,

    CHVALINVAL,
    CHVALINVAL,
    CHVALINVAL,
    CHVALINVAL,
    CHVALINVAL,
    CHVALINVAL,
    CHVALINVAL,
    CHVALINVAL,

    CHVALINVAL,
    CHVALINVAL,
    CHVALINVAL,
    CHVALINVAL,
    CHVALINVAL,
    CHVALINVAL,
    CHVALINVAL,
    CHVALINVAL,

    CHVALINVAL,
    CHVALINVAL,
    CHVALINVAL,
    CHVALINVAL,
    CHVALINVAL,
    CHVALINVAL,
    CHVALINVAL,
    CHVALINVAL,

    CHVALINVAL,
    CHVALINVAL,
    CHVALINVAL,
    CHVALINVAL,
    CHVALINVAL,
    CHVALINVAL,
    CHVALINVAL,
    CHVALINVAL,

    CHVALINVAL,
    CHVALINVAL,
    CHVALINVAL,
    CHVALINVAL,
    CHVALINVAL,
    CHVALINVAL,
    CHVALINVAL,
    CHVALINVAL,
};

/* calculate hash value for symbol name */
static __inline__ uint16_t
zcchashsym(char *name)
{
    uint16_t ret;
    long     val;
    int      ch;
    int      tmp = *name++;

    ch = chvaltab[tmp];
    while (ch) {
        val += ch;
        tmp = *name++;
        ch = chvaltab[tmp];
    }
    ret = val & 0xffff;

    return ret;
}

/* add symbol into lookup hash table */
static __inline__ void
zccaddsym(struct zccsym *sym)
{
    uint16_t       val = zcchashsym(sym->name);

    sym->prev = NULL;
    sym->next = symhash[val];
    if (sym->next) {
        sym->next->prev = sym;
    }
    symhash[val] = sym;

    return;
}

/* search hash table for symbol */
static __inline__ struct zccsym *
zccfindsym(char *name)
{
    char          *ptr = name;
    long           val = zcchashsym(name);
    struct zccsym *sym = symhash[val];

    while (sym) {
        if (!strcmp(ptr, sym->name)) {

            return sym;
        }
        sym = sym->next;
    }

    return sym;
}

/* remove symbol from hash table */
static __inline__ void
zccrmsym(struct zccsym *sym)
{
    uint16_t val;

    if (!sym->prev) {
        val = zcchashsym(sym->name);
        symhash[val] = sym->next;
    } else {
        sym->prev->next = sym->next;
    }
    if (sym->next) {
        sym->next->prev = sym->prev;
    }

    return;
}

/* calculate hash value for type name */
static __inline__ uint16_t
zcchashtype(char *name)
{
    uint16_t ret;
    long     val;
    int      ch;
    int      tmp = *name++;

    ch = chvaltab[tmp];
    while (ch) {
        val += ch;
        tmp = *name++;
        ch = chvaltab[tmp];
    }
    ret = val & 0xffff;

    return ret;
}

/* add type into lookup hash table */
static __inline__ void
zccaddtype(struct zcctoken *token)
{
    uint16_t val = zcchashtype(token->str);

    token->prev = NULL;
    token->next = typehash[val];
    if (token->next) {
        token->next->prev = token;
    }
    typehash[val] = token;

    return;
}

/* search hash table for type */
__inline__ long
zccfindtype(char *name)
{
    char            *ptr = name;
    long             val = zcchashtype(name);
    struct zcctoken *token = typehash[val];
    long             type = ZCC_NONE;

    while (token) {
        if (!strcmp(ptr, token->str)) {
            type = token->type;

            break;
        }
        token = token->next;
    }

    return type;
}

/* remove type from hash table */
static __inline__ void
zccrmtype(struct zcctoken *token)
{
    uint16_t val;

    if (!token->prev) {
        val = zcchashtype(token->str);
        typehash[val] = token->next;
    } else {
        token->prev->next = token->next;
    }
    if (token->next) {
        token->next->prev = token->prev;
    }

    return;
}

#if (NEWHASH)

void
zccaddid(struct hashstr **tab, char *str, long val)
{
    int             ch;
    long            key = 0;
    char           *ptr;
    long            sz = 8;
    long            n = 0;
    struct hashstr *item;

//    fprintf(stderr, "%s - ", str);
    if ((*str) && (isalpha(*str) || *str == '_')) {
        item = calloc(1, sizeof(struct hashstr));
        item->str = malloc(sz);
        ptr = item->str;
        ch = chvaltab[(int)(*str)];
        key <<= 6;
        *ptr++ = *str++;
        n++;
        key += ch;
        while ((*str) && (isalnum(*str) || *str == '_')) {
            ch = chvaltab[(int)(*str)];
            key <<= 6;
            if (n == sz) {
                sz <<= 1;
                item->str = realloc(item->str, sz);
                ptr = &item->str[n];
            }
            *ptr++ = *str++;
            n++;
            key += ch;
        }
        *ptr = '\0';
        key &= (NSTRHASH - 1);
        item->val = val;
        item->next = tab[key];
        tab[key] = item;
    }
//    fprintf(stderr, "%ld\n", key);

    return;
}

long
zccfindid(struct hashstr **tab, char *str)
{
    int             ch;
    long            key = 0;
    char           *ptr = str;
    long            n = 0;
    struct hashstr *item = NULL;
    long            ret = ZCC_NONE;

//    fprintf(stderr, "%s - ", str);
    if ((*str) && (isalpha(*str) || *str == '_')) {
        ch = chvaltab[(int)(*str)];
        key <<= 6;
        str++;
        n++;
        key += ch;
        while ((*str) && (isalnum(*str) || *str == '_')) {
            ch = chvaltab[(int)(*str)];
            key <<= 6;
            n++;
            str++;
            key += ch;
        }
        key &= (NSTRHASH - 1);
        item = tab[key];
        while ((item) && strncmp(item->str, ptr, n)) {
            item = item->next;
        }
        if (item) {
            ret = item->val;
        }
    }
    if (item) {
//        fprintf(stderr, "%ld (%ld, %ld: %s)\n", key, ret, n, item->str);
    }

    return ret;
}

#else

void
zccaddid(struct hashstr *tab, char *str, long val)
{
    int             ch;
    struct hashstr *ptr1;
    struct hashstr *ptr2 = NULL;

    if ((*str) && (isalpha(*str) || *str == '_')) {
        ch = chvaltab[(int)(*str)];
        str++;
        ptr1 = &tab[ch];
        while ((*str) && (isalnum(*str) || *str == '_')) {
            ch = chvaltab[(int)(*str)];
            ptr2 = ptr1->ptr;
            str++;
            if (!ptr2) {
                ptr2 = calloc(NSTRTABLVL, sizeof(struct hashstr));
                ptr1->ptr = ptr2;
            }
            ptr1 = &ptr2[ch];
        }
        if (ptr1) {
            ptr1->val = val;
        }
    }

    return;
}

long
zccfindid(struct hashstr *tab, char *str)
{
    int             ch;
    long            ret = ZCC_NONE;
    struct hashstr *ptr1;
    struct hashstr *ptr2 = NULL;

    if ((*str) && (isalpha(*str) || *str == '_')) {
        ch = chvaltab[(int)(*str)];
        str++;
        ptr1 = &tab[ch];
        if (!ptr1->ptr) {

            return ZCC_NONE;
        }
        while ((*str) && (isalnum(*str) || *str == '_')) {
            ch = chvaltab[(int)(*str)];
            ptr2 = ptr1->ptr;
            str++;
            if (!ptr2) {

                return ZCC_NONE;
            }
            ptr1 = &ptr2[ch];
        }
        if (ptr1) {
            ret = ptr1->val;
        }
    }

    return ret;
}

#endif

