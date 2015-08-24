#include <ctype.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <zcc/zcc.h>
#include <stdio.h>

#define NSYMHASH   65536
#define NTYPEHASH  65536
#if (NEWHASH)
#define NSTRHASH   4096
#else
#define NSTRTABLVL 64
#endif

static struct zccsym   *symhash[NSYMHASH];
static struct zcctoken *typehash[NTYPEHASH];

#if (NEWHASH)
struct hashstr *qualhash[NSTRHASH];
struct hashstr *preprochash[NSTRHASH];
struct hashstr *tunehash[NSTRHASH];
struct hashstr *atrhash[NSTRHASH];
#else
struct hashstr qualhash[NSTRTABLVL];
struct hashstr preprochash[NSTRTABLVL];
struct hashstr tunehash[NSTRTABLVL];
struct hashstr atrhash[NSTRTABLVL];
#endif

/*
 * ISO 8859-1 value compression table
 * Characters for identifier names have codes 0x00..0x3f
 * Other characters are [currently] marked invalid
 */
#define CH_INVAL 0xffff
static uint16_t chvaltab[256] =
{
    /* ASCII values */

    /* 0x00..0x07 */
    CH_INVAL, // NUL
    CH_INVAL, // SOH
    CH_INVAL, // STX
    CH_INVAL, // ETX
    CH_INVAL, // EOT
    CH_INVAL, // ENQ
    CH_INVAL, // ACK
    CH_INVAL, // BEL
    /* 0x08..0x0f */
    CH_INVAL, // BS
    CH_INVAL, // HT
    CH_INVAL, // LF
    CH_INVAL, // VT
    CH_INVAL, // FF
    CH_INVAL, // CR
    CH_INVAL, // SO
    CH_INVAL, // SI
    /* 0x10..0x17 */
    CH_INVAL, // DLE
    CH_INVAL, // DC1
    CH_INVAL, // DC2
    CH_INVAL, // DC3
    CH_INVAL, // DC4
    CH_INVAL, // NAK
    CH_INVAL, // SYN
    CH_INVAL, // ETB
    /* 0x18..0x1f */
    CH_INVAL, // CAN
    CH_INVAL, // EM
    CH_INVAL, // SUB
    CH_INVAL, // ESC
    CH_INVAL, // FS
    CH_INVAL, // GS
    CH_INVAL, // RS
    CH_INVAL, // US
    /* 0x20..0x27 */
    CH_INVAL, // SPACE
    CH_INVAL, // !
    CH_INVAL, // "
    CH_INVAL, // #
    CH_INVAL, // $
    CH_INVAL, // %
    CH_INVAL, // &
    CH_INVAL, // '
    /* 0x28..0x2f */
    CH_INVAL, // (
    CH_INVAL, // )
    CH_INVAL, // *
    CH_INVAL, // +
    CH_INVAL, // ,
    CH_INVAL, // -
    CH_INVAL, // .
    CH_INVAL, // /
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
    CH_INVAL, // :
    CH_INVAL, // ;
    CH_INVAL, // <
    CH_INVAL, // =
    CH_INVAL, // >
    CH_INVAL, // ?
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
    CH_INVAL, // [
    CH_INVAL, // '\'
    CH_INVAL, // ]
    CH_INVAL, // ^
    0x25,       // _
    /* 0x60..0x67 */
    CH_INVAL, // `
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
    CH_INVAL, // {
    CH_INVAL, // |
    CH_INVAL, // }
    CH_INVAL, // ~
    CH_INVAL, // DEL

    /* high 8-bit half (invalid in identifier names) */

    CH_INVAL,
    CH_INVAL,
    CH_INVAL,
    CH_INVAL,
    CH_INVAL,
    CH_INVAL,
    CH_INVAL,
    CH_INVAL,

    CH_INVAL,
    CH_INVAL,
    CH_INVAL,
    CH_INVAL,
    CH_INVAL,
    CH_INVAL,
    CH_INVAL,
    CH_INVAL,

    CH_INVAL,
    CH_INVAL,
    CH_INVAL,
    CH_INVAL,
    CH_INVAL,
    CH_INVAL,
    CH_INVAL,
    CH_INVAL,

    CH_INVAL,
    CH_INVAL,
    CH_INVAL,
    CH_INVAL,
    CH_INVAL,
    CH_INVAL,
    CH_INVAL,
    CH_INVAL,

    CH_INVAL,
    CH_INVAL,
    CH_INVAL,
    CH_INVAL,
    CH_INVAL,
    CH_INVAL,
    CH_INVAL,
    CH_INVAL,

    CH_INVAL,
    CH_INVAL,
    CH_INVAL,
    CH_INVAL,
    CH_INVAL,
    CH_INVAL,
    CH_INVAL,
    CH_INVAL,

    CH_INVAL,
    CH_INVAL,
    CH_INVAL,
    CH_INVAL,
    CH_INVAL,
    CH_INVAL,
    CH_INVAL,
    CH_INVAL,

    CH_INVAL,
    CH_INVAL,
    CH_INVAL,
    CH_INVAL,
    CH_INVAL,
    CH_INVAL,
    CH_INVAL,
    CH_INVAL,

    CH_INVAL,
    CH_INVAL,
    CH_INVAL,
    CH_INVAL,
    CH_INVAL,
    CH_INVAL,
    CH_INVAL,
    CH_INVAL,

    CH_INVAL,
    CH_INVAL,
    CH_INVAL,
    CH_INVAL,
    CH_INVAL,
    CH_INVAL,
    CH_INVAL,
    CH_INVAL,

    CH_INVAL,
    CH_INVAL,
    CH_INVAL,
    CH_INVAL,
    CH_INVAL,
    CH_INVAL,
    CH_INVAL,
    CH_INVAL,

    CH_INVAL,
    CH_INVAL,
    CH_INVAL,
    CH_INVAL,
    CH_INVAL,
    CH_INVAL,
    CH_INVAL,
    CH_INVAL,

    CH_INVAL,
    CH_INVAL,
    CH_INVAL,
    CH_INVAL,
    CH_INVAL,
    CH_INVAL,
    CH_INVAL,
    CH_INVAL,

    CH_INVAL,
    CH_INVAL,
    CH_INVAL,
    CH_INVAL,
    CH_INVAL,
    CH_INVAL,
    CH_INVAL,
    CH_INVAL,

    CH_INVAL,
    CH_INVAL,
    CH_INVAL,
    CH_INVAL,
    CH_INVAL,
    CH_INVAL,
    CH_INVAL,
    CH_INVAL,

    CH_INVAL,
    CH_INVAL,
    CH_INVAL,
    CH_INVAL,
    CH_INVAL,
    CH_INVAL,
    CH_INVAL,
    CH_INVAL,
};

/* calculate hash value for symbol name */
uint16_t
zcchashsym(char *name)
{
    uint16_t ret;
    long     val = 0;
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
void
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
struct zccsym *
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
void
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
uint16_t
zcchashtype(char *name)
{
    uint16_t ret;
    long     val = 0;
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
void
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
long
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
void
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
    long            len = 0;
    struct hashstr *item;
    void           *mptr;

    if ((*str) && (isalpha(*str) || *str == '_')) {
        item = calloc(1, sizeof(struct hashstr));
        item->str = malloc(sz);
        ptr = item->str;
        ch = chvaltab[(int)(*str)];
        key <<= 3;
        *ptr++ = *str++;
        len++;
        key += ch;
        while ((*str) && (isalnum(*str) || *str == '_')) {
            ch = chvaltab[(int)(*str)];
            key <<= 3;
            if (len == sz) {
                sz <<= 1;
                mptr = realloc(item->str, sz);
                if (!mptr) {
                    free(item->str);
                    fprintf(stderr, "out of memory\n");

                    exit(1);
                }
                item->str = mptr;
                ptr = &item->str[len];
            }
            *ptr++ = *str++;
            len++;
            key += ch;
        }
        *ptr = '\0';
        key &= (NSTRHASH - 1);
        item->val = val;
        item->next = tab[key];
        tab[key] = item;
    }

    return;
}

long
zccfindid(struct hashstr **tab, char *str)
{
    int             ch;
    long            key = 0;
    char           *ptr = str;
    long            len = 0;
    struct hashstr *item = NULL;
    long            ret = ZCC_NONE;

    if ((*str) && (isalpha(*str) || *str == '_')) {
        ch = chvaltab[(int)(*str)];
        key <<= 3;
        str++;
        len++;
        key += ch;
        while ((*str) && (isalnum(*str) || *str == '_')) {
            ch = chvaltab[(int)(*str)];
            key <<= 3;
            len++;
            str++;
            key += ch;
        }
        key &= (NSTRHASH - 1);
        item = tab[key];
        while ((item) && strncmp(item->str, ptr, len)) {
            item = item->next;
        }
        if (item && !item->str[len]) {
            ret = item->val;
        }
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

