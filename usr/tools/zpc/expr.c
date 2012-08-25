#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <zpc/zpc.h>
#include <zpc/op.h>

#define TOKENSTRLEN 128

#define todec(c)    zpcdectab[(int)(c)]
#define tohex(c)    zpchextab[(int)(c)]
#define tooct(c)    zpcocttab[(int)(c)]
#define todecflt(c) zpcdecflttab[(int)(c)]
#define tohexflt(c) zpchexflttab[(int)(c)]
#define tooctflt(c) zpcoctflttab[(int)(c)]
#define todecdbl(c) zpcdecdbltab[(int)(c)]
#define tohexdbl(c) zpchexdbltab[(int)(c)]
#define tooctdbl(c) zpcoctdbltab[(int)(c)]
#define tobin(c)    ((c) == '0' ? 0 : 1)
#define tobinflt(c) ((c) == '0' ? 0.0f : 1.0f)
#define tobindbl(c) ((c) == '0' ? 0.0 : 1.0)
#define isbdigit(c) ((c) == '0' || (c) == '1')
#define isodigit(c) ((c) >= '0' && (c) <= '7')

extern long             zpcradix;

#define OPERRTOL  0x80000000
#define zpccopprec(tp)                                                 \
    (zpccopprectab[(tp)->type] & ~OPERRTOL)
#define zpccopisrtol(tp)                                               \
    (zpccopprectab[(tp)->type] & OPERRTOL)
static uint8_t          zpcdectab[256];
static uint8_t          zpchextab[256];
static uint8_t          zpcocttab[256];
static float            zpcdecflttab[256];
static float            zpchexflttab[256];
static float            zpcoctflttab[256];
static double           zpcdecdbltab[256];
static double           zpchexdbltab[256];
static double           zpcoctdbltab[256];
uint8_t                 zpcoperchartab[256];
static long             zpccopprectab[ZPCNOPER];
static long             zpccopnargtab[ZPCNOPER];
static zpccop_t        *zpcevaltab[ZPCNOPER];
struct zpctoken        *zpcoperstk;
struct zpctoken        *zpcoperstktop;
struct zpctoken        *zpctokenqueue;
struct zpctoken        *zpctokentail;
struct zpctoken        *zpcparsequeue;
struct zpctoken        *zpcparsetail;

void
zpcinitconvtab(void)
{
    /* integral tables */
    zpcdectab['0'] = 0x00;
    zpcdectab['1'] = 0x01;
    zpcdectab['2'] = 0x02;
    zpcdectab['3'] = 0x03;
    zpcdectab['4'] = 0x04;
    zpcdectab['5'] = 0x05;
    zpcdectab['6'] = 0x06;
    zpcdectab['7'] = 0x07;
    zpcdectab['8'] = 0x08;
    zpcdectab['9'] = 0x09;
    zpchextab['0'] = 0x00;
    zpchextab['1'] = 0x01;
    zpchextab['2'] = 0x02;
    zpchextab['3'] = 0x03;
    zpchextab['4'] = 0x04;
    zpchextab['5'] = 0x05;
    zpchextab['6'] = 0x06;
    zpchextab['7'] = 0x07;
    zpchextab['8'] = 0x08;
    zpchextab['9'] = 0x09;
    zpchextab['a'] = 0x0a;
    zpchextab['b'] = 0x0b;
    zpchextab['c'] = 0x0c;
    zpchextab['d'] = 0x0d;
    zpchextab['e'] = 0x0e;
    zpchextab['f'] = 0x0f;
    zpchextab['A'] = 0x0a;
    zpchextab['B'] = 0x0b;
    zpchextab['C'] = 0x0c;
    zpchextab['D'] = 0x0d;
    zpchextab['E'] = 0x0e;
    zpchextab['F'] = 0x0f;
    zpcocttab['0'] = 0x00;
    zpcocttab['1'] = 0x01;
    zpcocttab['2'] = 0x02;
    zpcocttab['3'] = 0x03;
    zpcocttab['4'] = 0x04;
    zpcocttab['5'] = 0x05;
    zpcocttab['6'] = 0x06;
    zpcocttab['7'] = 0x07;
    /* float tables */
    zpcdecflttab['0'] = 0.0f;
    zpcdecflttab['1'] = 1.0f;
    zpcdecflttab['2'] = 2.0f;
    zpcdecflttab['3'] = 3.0f;
    zpcdecflttab['4'] = 4.0f;
    zpcdecflttab['5'] = 5.0f;
    zpcdecflttab['6'] = 6.0f;
    zpcdecflttab['7'] = 7.0f;
    zpcdecflttab['8'] = 8.0f;
    zpcdecflttab['9'] = 9.0f;
    zpchexflttab['0'] = 0.0f;
    zpchexflttab['1'] = 1.0f;
    zpchexflttab['2'] = 2.0f;
    zpchexflttab['3'] = 3.0f;
    zpchexflttab['4'] = 4.0f;
    zpchexflttab['5'] = 5.0f;
    zpchexflttab['6'] = 6.0f;
    zpchexflttab['7'] = 7.0f;
    zpchexflttab['8'] = 8.0f;
    zpchexflttab['9'] = 9.0f;
    zpchexflttab['a'] = 10.0f;
    zpchexflttab['b'] = 11.0f;
    zpchexflttab['c'] = 12.0f;
    zpchexflttab['d'] = 13.0f;
    zpchexflttab['e'] = 14.0f;
    zpchexflttab['f'] = 15.0f;
    zpchexflttab['A'] = 10.0f;
    zpchexflttab['B'] = 11.0f;
    zpchexflttab['C'] = 12.0f;
    zpchexflttab['D'] = 13.0f;
    zpchexflttab['E'] = 14.0f;
    zpchexflttab['F'] = 15.0f;
    zpcoctflttab['0'] = 0.0f;
    zpcoctflttab['1'] = 1.0f;
    zpcoctflttab['2'] = 2.0f;
    zpcoctflttab['3'] = 3.0f;
    zpcoctflttab['4'] = 4.0f;
    zpcoctflttab['5'] = 5.0f;
    zpcoctflttab['6'] = 6.0f;
    zpcoctflttab['7'] = 7.0f;
    /* double tables */
    zpcdecdbltab['0'] = 0.0;
    zpcdecdbltab['1'] = 1.0;
    zpcdecdbltab['2'] = 2.0;
    zpcdecdbltab['3'] = 3.0;
    zpcdecdbltab['4'] = 4.0;
    zpcdecdbltab['5'] = 5.0;
    zpcdecdbltab['6'] = 6.0;
    zpcdecdbltab['7'] = 7.0;
    zpcdecdbltab['8'] = 8.0;
    zpcdecdbltab['9'] = 9.0;
    zpchexdbltab['0'] = 0.0;
    zpchexdbltab['1'] = 1.0;
    zpchexdbltab['2'] = 2.0;
    zpchexdbltab['3'] = 3.0;
    zpchexdbltab['4'] = 4.0;
    zpchexdbltab['5'] = 5.0;
    zpchexdbltab['6'] = 6.0;
    zpchexdbltab['7'] = 7.0;
    zpchexdbltab['8'] = 8.0;
    zpchexdbltab['9'] = 9.0;
    zpchexdbltab['a'] = 10.0;
    zpchexdbltab['b'] = 11.0;
    zpchexdbltab['c'] = 12.0;
    zpchexdbltab['d'] = 13.0;
    zpchexdbltab['e'] = 14.0;
    zpchexdbltab['f'] = 15.0;
    zpchexdbltab['A'] = 10.0;
    zpchexdbltab['B'] = 11.0;
    zpchexdbltab['C'] = 12.0;
    zpchexdbltab['D'] = 13.0;
    zpchexdbltab['E'] = 14.0;
    zpchexdbltab['F'] = 15.0;
    zpcoctdbltab['0'] = 0.0;
    zpcoctdbltab['1'] = 1.0;
    zpcoctdbltab['2'] = 2.0;
    zpcoctdbltab['3'] = 3.0;
    zpcoctdbltab['4'] = 4.0;
    zpcoctdbltab['5'] = 5.0;
    zpcoctdbltab['6'] = 6.0;
    zpcoctdbltab['7'] = 7.0;

    return;
}

void
zpcinitcoptab(void)
{
    /* precedences */
    zpccopprectab[ZPCNOT] = OPERRTOL | 8;
    zpccopprectab[ZPCINC] = 8;
    zpccopprectab[ZPCDEC] = 8;
    zpccopprectab[ZPCSHR] = 7;
    zpccopprectab[ZPCSHL] = 7;
    zpccopprectab[ZPCAND] = 6;
    zpccopprectab[ZPCXOR] = OPERRTOL | 5;
    zpccopprectab[ZPCOR] = 4;
    zpccopprectab[ZPCMUL] = 3;
    zpccopprectab[ZPCDIV] = 3;
    zpccopprectab[ZPCMOD] = 3;
    zpccopprectab[ZPCADD] = OPERRTOL | 2;
    zpccopprectab[ZPCSUB] = 2;
    zpccopprectab[ZPCASSIGN] = 1;
    /* # of arguments */
    zpccopnargtab[ZPCNOT] = 1;
    zpccopnargtab[ZPCINC] = 1;
    zpccopnargtab[ZPCDEC] = 1;
    zpccopnargtab[ZPCSHL] = 2;
    zpccopnargtab[ZPCSHR] = 2;
    zpccopnargtab[ZPCSHRA] = 2;
    zpccopnargtab[ZPCAND] = 2;
    zpccopnargtab[ZPCXOR] = 2;
    zpccopnargtab[ZPCOR] = 2;
    zpccopnargtab[ZPCMUL] = 2;
    zpccopnargtab[ZPCDIV] = 2;
    zpccopnargtab[ZPCMOD] = 2;
    zpccopnargtab[ZPCADD] = 2;
    zpccopnargtab[ZPCSUB] = 2;
    /* evaluation functions */
    zpcevaltab[ZPCNOT] = not64;
    zpcevaltab[ZPCINC] = inc64;
    zpcevaltab[ZPCDEC] = dec64;
    zpcevaltab[ZPCSHL] = shl64;
    zpcevaltab[ZPCSHR] = shrl64;
    zpcevaltab[ZPCSHRA] = shr64;
    zpcevaltab[ZPCAND] = and64;
    zpcevaltab[ZPCXOR] = xor64;
    zpcevaltab[ZPCOR] = or64;
    zpcevaltab[ZPCMUL] = mul64;
    zpcevaltab[ZPCDIV] = div64;
    zpcevaltab[ZPCMOD] = mod64;
    zpcevaltab[ZPCADD] = add64;
    zpcevaltab[ZPCSUB] = sub64;

    return;
}

void zpcinitcop(void)
{
    /* lookup table */
    zpcoperchartab['~'] = '~';
    zpcoperchartab['&'] = '&';
    zpcoperchartab['|'] = '|';
    zpcoperchartab['^'] = '^';
    zpcoperchartab['<'] = '<';
    zpcoperchartab['>'] = '>';
    zpcoperchartab['+'] = '+';
    zpcoperchartab['-'] = '-';
    zpcoperchartab['*'] = '*';
    zpcoperchartab['/'] = '/';
    zpcoperchartab['%'] = '%';
    zpcoperchartab['='] = '%';
}

void
exprinit(void)
{
    zpcinitconvtab();
    zpcinitcoptab();
    zpcinitcop();

    return;
}

#if 0
static __inline__ void
zpcqueueexpr(struct zpctoken *token,
             struct zpctoken **queue, struct zpctoken **tail)
{
    token->next = NULL;
    if (!*queue) {
        token->prev = NULL;
        *queue = token;
    } else if (*tail) {
        token->prev = *tail;
        (*tail)->next = token;
        *tail = token;
    } else {
        (*queue)->next = token;
        token->prev = *queue;
        *tail = token;
    }

    return;
}
#endif

void
zpcqueuetoken(struct zpctoken *token,
              struct zpctoken **queue, struct zpctoken **tail)
{
    if (token) {
        token->next = NULL;
        if (!*queue) {
            token->prev = NULL;
            *queue = token;
        } else if (*tail) {
            token->prev = *tail;
            (*tail)->next = token;
            *tail = token;
        } else {
            token->prev = *queue;
            (*queue)->next = token;
            *tail = token;
        }
    }

    return;
}

static __inline__ void
zpcpushtoken(struct zpctoken *token, struct zpctoken **stack)
{
    if ((token) && (stack)) {
        token->prev = NULL;
        token->next = *stack;
        *stack = token;
    }

    return;
}

static __inline__ struct zpctoken *
zpcpoptoken(struct zpctoken **stack)
{
    struct zpctoken *token;

    if (stack) {
        token = *stack;
        if (token) {
            *stack = token->next;
        }
    }

    return token;
}

#if 0
void
zpcgetfloat(struct zpctoken *token, const char *str, char **retstr)
{
    char  *ptr = (char *)str;
    float  flt = 0.0;
    float  div;

    if (*ptr == '0') {
        ptr++;
        if (*ptr == 'x' || *ptr == 'X') {
            /* hexadecimal value */
            ptr++;
            while (*ptr != '.' && isxdigit(*ptr)) {
                flt *= 16.0f;
                flt += tohexflt(*ptr);
                ptr++;
            }
            ptr++;
            div = 16.0f;
            while (isxdigit(*ptr)) {
                flt += tohexflt(*ptr) / div;
                div *= 16.0f;
                ptr++;
            }
            token->radix = 16;
            token->data.f32 = flt;
        } else if (*ptr == 'b' || *ptr == 'B') {
            /* binary value */
            ptr++;
            while (*ptr != '.' && isbdigit(*ptr)) {
                flt *= 2.0f;
                flt += tobinflt(*ptr);
                ptr++;
            }
            ptr++;
            div = 2.0f;
            while (isbdigit(*ptr)) {
                flt += tohexflt(*ptr) / div;
                div *= 2.0f;
                ptr++;
            }
            token->radix = 2;
            token->data.f32 = flt;
        } else {
            /* octal value */
            ptr++;
            while (*ptr != '.' && isodigit(*ptr)) {
                flt *= 8.0f;
                flt += tobinflt(*ptr);
                ptr++;
            }
            ptr++;
            div = 8.0f;
            while (isodigit(*ptr)) {
                flt += tohexflt(*ptr) / div;
                div *= 8.0f;
                ptr++;
            }
            token->radix = 8;
            token->data.f32 = flt;
        }
    } else {
        /* decimal value */
        while (*ptr != '.' && isdigit(*ptr)) {
            flt *= 10.0f;
            flt += tobinflt(*ptr);
            ptr++;
        }
        ptr++;
        div = 10.0;
        while (isdigit(*ptr)) {
            flt += tohexflt(*ptr) / div;
            div *= 10.0;
            ptr++;
        }
        token->radix = 10;
        token->data.f32 = flt;
    }
    if (*ptr == 'f' || *ptr == 'F' || *ptr == ',') {
        ptr++;
    }
    *retstr = (char *)ptr;

    return;
}
#endif

void
zpcgetdouble(struct zpctoken *token, const char *str, char **retstr)
{
    char  *ptr = (char *)str;
    float  dbl = 0.0;
    float  div;

    if (*ptr == '0') {
        ptr++;
        if (*ptr == 'x' || *ptr == 'X') {
            /* hexadecimal value */
            ptr++;
            while (*ptr != '.' && isxdigit(*ptr)) {
                dbl *= 16.0;
                dbl += tohexdbl(*ptr);
                ptr++;
            }
            ptr++;
            div = 16.0;
            while (isxdigit(*ptr)) {
                dbl += tohexdbl(*ptr) / div;
                div *= 16.0;
                ptr++;
            }
            token->radix = 16;
            token->data.f64 = dbl;
        } else if (*ptr == 'b' || *ptr == 'B') {
            /* binary value */
            ptr++;
            while (*ptr != '.' && isbdigit(*ptr)) {
                dbl *= 2.0;
                dbl += tobindbl(*ptr);
                ptr++;
            }
            ptr++;
            div = 2.0;
            while (isbdigit(*ptr)) {
                dbl += tohexdbl(*ptr) / div;
                div *= 2.0;
                ptr++;
            }
            token->radix = 2;
            token->data.f64 = dbl;
        } else {
            /* octal value */
            ptr++;
            while (*ptr != '.' && isodigit(*ptr)) {
                dbl *= 8.0;
                dbl += tobindbl(*ptr);
                ptr++;
            }
            ptr++;
            div = 8.0;
            while (isodigit(*ptr)) {
                dbl += tohexdbl(*ptr) / div;
                div *= 8.0;
                ptr++;
            }
            token->radix = 8;
            token->data.f64 = dbl;
        }
    } else {
        /* decimal value */
        while (*ptr != '.' && isdigit(*ptr)) {
            dbl *= 10.0;
            dbl += tobindbl(*ptr);
            ptr++;
        }
        ptr++;
        div = 10.0;
        while (isdigit(*ptr)) {
            dbl += tohexdbl(*ptr) / div;
            div *= 10.0;
            ptr++;
        }
        token->radix = 10;
        token->data.f64 = dbl;
    }
    if (*ptr == ',') {
        ptr++;
    }
    *retstr = (char *)ptr;

    return;
}

void
zpcgetint64(struct zpctoken *token, const char *str, char **retstr)
{
    char    *ptr = (char *)str;
    int64_t  i64 = 0;

    if (*ptr == '0') {
        ptr++;
        if (*ptr == 'x' || *ptr == 'X') {
            /* hexadecimal value */
            ptr++;
            while (isxdigit(*ptr)) {
                i64 <<= 4;
                i64 += tohex(*ptr);
                ptr++;
            }
            token->radix = 16;
            token->data.ui64.i64 = i64;
        } else if (*ptr == 'b' || *ptr == 'B') {
            /* binary value */
            ptr++;
            while (isxdigit(*ptr)) {
                i64 <<= 1;
                i64 += tobin(*ptr);
                ptr++;
            }
            token->radix = 2;
            token->data.ui64.i64 = i64;
        } else {
            /* octal value */
            ptr++;
            while (isxdigit(*ptr)) {
                i64 <<= 3;
                i64 += tooct(*ptr);
                ptr++;
            }
            token->radix = 8;
            token->data.ui64.i64 = i64;
        }
    } else if (isdigit(*ptr)) {
        /* decimal value */
        while (isdigit(*ptr)) {
            i64 *= 10;
            i64 += todec(*ptr);
            ptr++;
        }
        token->radix = 10;
        token->data.ui64.i64 = i64;
#if 0
    } else if (isxdigit(*ptr)) {
        while (isxdigit(*ptr)) {
            i64 <<= 4;
            i64 += tohex(*ptr);
            ptr++;
        }
        token->data.ui64.i64 = i64;
#endif
    }
    if (*ptr == ',') {
        ptr++;
    }
    token->type = ZPCINT64;
    *retstr = (char *)ptr;

    return;
}

void
zpcgetuint64(struct zpctoken *token, const char *str, char **retstr)
{
    char     *ptr = (char *)str;
    uint64_t  u64 = 0;

    if (*ptr == '0') {
        ptr++;
        if (*ptr == 'x' || *ptr == 'X') {
            /* hexadecimal value */
            ptr++;
            while (isxdigit(*ptr)) {
                u64 <<= 4;
                u64 += tohex(*ptr);
                ptr++;
            }
            token->radix = 16;
            token->data.ui64.u64 = u64;
        } else if (*ptr == 'b' || *ptr == 'B') {
            /* binary value */
            ptr++;
            while (isxdigit(*ptr)) {
                u64 <<= 1;
                u64 += tobin(*ptr);
                ptr++;
            }
            token->radix = 2;
            token->data.ui64.u64 = u64;
        } else {
            /* octal value */
            ptr++;
            while (isxdigit(*ptr)) {
                u64 <<= 3;
                u64 += tooct(*ptr);
                ptr++;
            }
            token->radix = 8;
            token->data.ui64.u64 = u64;
        }
    } else if (isdigit(*ptr)) {
        /* decimal value */
        while (isdigit(*ptr)) {
            u64 *= 10;
            u64 += todec(*ptr);
            ptr++;
        }
        token->radix = 10;
        token->data.ui64.u64 = u64;
#if 0
    } else if (isxdigit(*ptr)) {
        while (isxdigit(*ptr)) {
            u64 <<= 4;
            u64 += tohex(*ptr);
            ptr++;
        }
        token->data.ui64.u64 = u64;
#endif
    }
    if (*ptr == 'u' || *ptr == 'U' || *ptr == ',') {
        ptr++;
    }
    token->type = ZPCUINT64;
    *retstr = (char *)ptr;

    return;
}

void
zpcgetstr(struct zpctoken *token, const char *str, char **retstr)
{
    size_t      slen = token->slen;
    size_t      len = 0;
    const char *ptr = str;
    char       *dest = token->str;

    if (!slen) {
        slen = TOKENSTRLEN;
        dest = token->str = malloc(slen);
        token->slen = slen;
    }
    if (*ptr == '"') {
        while ((slen--) && (*str == '_' || isalnum(*str))) {
            fprintf(stderr, "CHAR: %x\n", *str);
            *dest++ = *ptr++;
            len++;
            if (!slen) {
                token->str = realloc(token->str, len << 1);
                token->slen = len << 1;
                dest = &token->str[len];
                slen = len;
            }
        }
        if (*str == '"') {
            *dest++ = '\"';
            *dest = '\0';
            ptr++;
        }
    }
    *retstr = (char *)ptr;
};

void
zpcgetvector(struct zpctoken *token, const char *str, char **retstr)
{
    char             *ptr = (char *)str;
    struct zpctoken **toktab;
    struct zpctoken  *token1;
    long              ndx = 0;
    long              type = 0;
    char             *cp;
    long              sign = 0;

    toktab = malloc(DEFAULTDIM * sizeof(struct zpctoken *));
    ptr++; /* skip the '<' */
    while ((*ptr) && *ptr != '>') {
        while (isspace(*ptr)) {
            ptr++;
        }
        if (*ptr) {
            token1 = calloc(1, sizeof(struct zpctoken));
            if (*ptr == '-') {
                sign = 1;
                ptr++;
            };
            cp = index(ptr, '.');
            if (cp) {
                type = ZPCDOUBLE;
                zpcgetdouble(token1, ptr, &ptr);
            } else {
                if (strstr(ptr, "uU")) {
                    if (sign) {
                        fprintf(stderr, "sign on unsigned number\n");
                        
                        return;
                    }
                }
                if (!type) {
                    if (sign) {
#if (SMARTTYPES)
                        token1->sign = ZPCUSERSIGNED;
#endif
                        type = ZPCINT64;
                    } else {
                        type = ZPCUINT64;
                    }
                }
#if (SMARTTYPES)
                if (type == ZPCUINT64) {
                    zpcgetuint64(token1, ptr, &ptr);
                } else {
                    zpcgetint64(token1, ptr, &ptr);
                }
                if (ZPCUSERSIGNED || token1->data.ui64.u64 <= INT64_MAX) {
                    token1->type = ZPCINT64;
                } else if (!sign) {
                    token1->type = ZPCUINT64;
                }
#else
                token1->type = type;
#endif
            }
            toktab[ndx++] = token1;
        }
        if (*ptr == ',') {
            ptr++;
        }
    }
    token->type = ZPCVECTOR;
    token->data.vector.ndim = ndx;
    token->data.vector.toktab = toktab;
    *retstr = ptr;

    return;
};

void
zpcgetoper(struct zpctoken *token, const char *str, char **retstr)
{
    char *ptr = (char *)str;

    token->str = malloc(TOKENSTRLEN);
    token->slen = TOKENSTRLEN;
    if (*ptr == '!') {
        ptr++;
        token->type = ZPCNOT;
        token->str[0] = '!';
        token->str[1] = '\0';
    } else if (*ptr == '&') {
        ptr++;
        token->type = ZPCAND;
        token->str[0] = '&';
        token->str[1] = '\0';
    } else if (*ptr == '|') {
        ptr++;
        token->type = ZPCOR;
        token->str[0] = '|';
        token->str[1] = '\0';
    } else if (*ptr == '^') {
        ptr++;
        token->type = ZPCXOR;
        token->str[0] = '^';
        token->str[1] = '\0';
    } else if (*ptr == '<' && ptr[1] == '<') {
        ptr += 2;
        token->type = ZPCSHL;
        token->str[0] = token->str[1] = '<';
        token->str[2] = '\0';
    } else if (*ptr == '>' && ptr[1] == '>') {
        if (ptr[2] == '>') {
            ptr += 3;
            token->type = ZPCSHRA;
            token->str[0] = token->str[1] = token->str[2] = '>';
            token->str[3] = '\0';
        } else {
            ptr += 2;
            token->type = ZPCSHR;
            token->str[0] = token->str[1] = '>';
            token->str[2] = '\0';
        }
    } else if (*ptr == '+') {
        if (ptr[1] == '+') {
            token->type = ZPCINC;
            ptr += 2;
            token->str[0] = token->str[1] = '+';
            token->str[2] = '\0';
        } else {
            token->type = ZPCADD;
            ptr++;
            token->str[0] = '+';
            token->str[1] = '\0';
        }
    } else if (*ptr == '-') {
        if (ptr[1] == '-') {
            token->type = ZPCDEC;
            ptr += 2;
            token->str[0] = token->str[1] = '-';
            token->str[2] = '\0';
        } else {
            token->type = ZPCSUB;
            ptr++;
            token->str[0] = '-';
            token->str[1] = '\0';
        }
    } else if (*ptr == '*') {
        ptr++;
        token->type = ZPCMUL;
        token->str[0] = '*';
        token->str[1] = '\0';
    } else if (*ptr == '/') {
        ptr++;
        token->type = ZPCDIV;
        token->str[0] = '/';
        token->str[1] = '\0';
    } else if (*ptr == '=') {
        ptr++;
        token->type = ZPCASSIGN;
        token->str[0] = '=';
        token->str[1] = '\0';
    } else if (*ptr == '%') {
        ptr++;
        token->type = ZPCMOD;
        token->str[0] = '%';
        token->str[1] = '\0';
    }
    token->param = zpccopnargtab[token->type];
    *retstr = (char *)ptr;
}

struct zpctoken *
zpcgettoken(const char *str, char **retstr)
{
    char            *ptr = (char *)str;
    struct zpctoken *token = NULL;
    char            *dec;
    long             sign = 0;
    long             radix;

    if (!*str) {

        return NULL;
    }
    while (isspace(*ptr)) {
        ptr++;
    }
    token = calloc(1, sizeof(struct zpctoken));
    if (*ptr == '-' && !isspace(ptr[1])) {
#if (SMARTTYPES)
        sign = ZPCUSERSIGNED;
#else
        sign = 1;
#endif
        ptr++;
    }
    if (*ptr == '-' || isxdigit(*ptr)) {
        token->str = calloc(1, TOKENSTRLEN);
        token->slen = TOKENSTRLEN;
        dec = index(ptr, '.');
        if(strstr(ptr, "uU")) {
#if (SMARTTYPES)
            if (sign == ZPCUSERSIGNED) {
                fprintf(stderr, "sign on unsigned number\n");

                return NULL;
            }
            sign = ZPCUSERUNSIGNED;
#else
            if (sign) {
                fprintf(stderr, "sign on unsigned number\n");

                return NULL;
            }
#endif
        }
        if (dec) {
//            token->type = ZPCDOUBLE;
            zpcgetdouble(token, ptr, &ptr);
            if (sign) {
                token->data.f64 = -token->data.f64;
                token->sign = sign;
            }
            sprintf(token->str, "%e", token->data.f64);
        } else if (!sign) {
            token->sign = 0;
            zpcgetuint64(token, ptr, &ptr);
#if (SMARTTYPES)
            if (sign == ZPCUSERSIGNED || token->data.ui64.u64 <= INT64_MAX) {
                token->type = ZPCINT64;
                token->sign = sign;
            } else {
                token->sign = ZPCUNSIGNED;
                token->type = ZPCUINT64;
            }
#else
//            token->type = ZPCUINT64;
#endif
            radix = token->radix;
            if (!radix) {
                radix = zpcradix;
            }
            switch (radix) {
                case 2:
                    zpcconvbinuint64(token->data.ui64.u64, token->str,
                                     TOKENSTRLEN);

                    break;
                case 8:
                    sprintf(token->str, "%llo", token->data.ui64.u64);

                    break;
                case 10:
                    sprintf(token->str, "%llu", token->data.ui64.u64);

                    break;
                case 16:
                default:
                    sprintf(token->str, "0x%llx", token->data.ui64.u64);

                    break;
            }
        } else {
//            token->type = ZPCINT64;
            zpcgetint64(token, ptr, &ptr);
            token->data.ui64.i64 = -token->data.ui64.i64;
#if (SMARTTYPES)
            token->sign = ZPCUSERSIGNED;
#else
            token->sign = 1;
#endif
            radix = token->radix;
            if (!radix) {
                radix = zpcradix;
            }
            switch (radix) {
                case 2:
                    zpcconvbinint64(token->data.ui64.i64, token->str,
                                    TOKENSTRLEN);

                    break;
                case 8:
                    sprintf(token->str, "%llo", token->data.ui64.i64);

                    break;
                case 10:
                    sprintf(token->str, "%lld", token->data.ui64.i64);

                    break;
                case 16:
                default:
                    sprintf(token->str, "0x%llx", token->data.ui64.i64);

                    break;
            }
        }
    } else if (*ptr == '"') {
        zpcgetstr(token, ptr, &ptr);
        token->type = ZPCSTRING;
    } else if (*ptr == '<' && isxdigit(ptr[1])) {
        zpcgetvector(token, ptr, &ptr);
    } else if (*ptr == '(' && isxdigit(ptr[1])) {
//        zpcgetcomplex(token, ptr, &ptr);
    } else if (zpcisoperchar(*ptr)) {
        zpcgetoper(token, ptr, &ptr);
    } else if (*ptr == '(') {
        token->type = ZPCLEFTPAREN;
        *token->str = '(';
        token->str[1] = '\0';
        token->len = 1;
        ptr++;
    } else if (*ptr == ')') {
        token->type = ZPCRIGHTPAREN;
        *token->str = ')';
        token->str[1] = '\0';
        token->len = 1;
        ptr++;
    } else {
        free(token);
        token = NULL;
    }
    if (token) {
        *retstr = (char *)ptr;
    }

    return token;
}

void
printtoken(struct zpctoken *token)
{
    fprintf(stderr, "TOKEN of type %lx: ", token->type);
    if (token->str) {
        fprintf(stderr, "%s\n", token->str);
    } else if (token->type == ZPCINT64) {
        fprintf(stderr, "%lld\n", token->data.ui64.i64);
    } else if (token->type == ZPCUINT64) {
        fprintf(stderr, "%llu\n", token->data.ui64.u64);
    } else if (token->type == ZPCDOUBLE) {
        fprintf(stderr, "%e\n", token->data.f64);
    } else if (token->type == ZPCLEFTPAREN) {
        fprintf(stderr, "(\n");
    } else if (token->type == ZPCRIGHTPAREN) {
        fprintf(stderr, ")\n");
    } else {
        fprintf(stderr, "\n");
    }
}

void
zpcprintqueue(struct zpctoken *queue)
{
    struct zpctoken *token = queue;

    while (token) {
        printtoken(token);
        token = token->next;
    }

    return;
}

struct zpctoken *
zpctokenize(const char *str)
{
    char            *ptr = (char *)str;
    struct zpctoken *queue = NULL;
    struct zpctoken *tail = NULL;
    struct zpctoken *token;

    token = zpcgettoken(ptr, &ptr);
    while (token) {
        zpcqueuetoken(token, &queue, &tail);
        token = zpcgettoken(ptr, &ptr);
    }
#if 0
    fprintf(stderr, "TOKENS: ");
    zpcprintqueue(zpctokenqueue);
#endif

    return queue;
};

/*
 * Dijkstra's shunting yard algorithm
 * - turns infix-format expressions into RPN queues
 * - https://en.wikipedia.org/wiki/Shunting-yard_algorithm
 */
struct zpctoken *
zpcparse(struct zpctoken *srcqueue)
{
    struct zpctoken *token;
    struct zpctoken *token1 = srcqueue;
    struct zpctoken *token2 = NULL;
    struct zpctoken *token3 = NULL;
    struct zpctoken *queue = NULL;
    struct zpctoken *tail = NULL;
    struct zpctoken *stack = NULL;

    while (token1) {
//        printtoken(token1);
        token = malloc(sizeof(struct zpctoken));
        token3 = token1->next;
        memcpy(token, token1, sizeof(struct zpctoken));
        token = token1;
        if (zpcisvalue(token)) {
            zpcqueuetoken(token, &queue, &tail);
        } else if (zpcisfunc(token)) {
            zpcpushtoken(token, &stack);
        } else if (zpcissep(token)) {
            token2 = stack;
            while ((token2) && token2->type != ZPCLEFTPAREN) {
                token2 = zpcpoptoken(&stack);
                zpcqueuetoken(token2, &queue, &tail);
                token2 = zpcpoptoken(&stack);
            }
            if ((token2) && token2->type == ZPCLEFTPAREN) {
                
                continue;
            } else {
                fprintf(stderr, "mismatched parentheses: %s\n", token2->str);
                
                return NULL;
            }
        } else if (zpcisoper(token)) {
            token2 = stack;
            while (zpcisoper(token2)) {
                if ((!zpccopisrtol(token)
                     && zpccopprec(token) <= zpccopprec(token2))
                    || zpccopprec(token) < zpccopprec(token2)) {
//                    fprintf(stderr, "POP: %s (%s)\n", token2->str, token->str);
                    token2 = zpcpoptoken(&stack);
                    zpcqueuetoken(token2, &queue, &tail);
                    token2 = stack;
                } else {

                    break;
                }
            }
//            fprintf(stderr, "PUSH: %s\n", token->str);
            zpcpushtoken(token, &stack);
        } else if (token->type == ZPCLEFTPAREN) {
            zpcpushtoken(token, &stack);
        } else if (token->type == ZPCRIGHTPAREN) {
            token2 = stack;
            while ((token2) && token2->type != ZPCLEFTPAREN) {
                token2 = zpcpoptoken(&stack);
                zpcqueuetoken(token2, &queue, &tail);
                token2 = stack;
            }
            if ((token2) && token2->type == ZPCLEFTPAREN) {
                token2 = zpcpoptoken(&stack);
            } else {
                if (token2) {
                    fprintf(stderr, "mismatched parentheses: %s\n",
                            token2->str);
                }
                
                return NULL;
            }
            if (zpcisfunc(stack)) {
                token2 = zpcpoptoken(&stack);
                zpcqueuetoken(token2, &queue, &tail);
            }
        }
        token1 = token3;
    }
#if 0
    fprintf(stderr, "QUEUE: ");
    zpcprintqueue(queue);
    fprintf(stderr, "STACK: ");
    zpcprintqueue(stack);
#endif
    do {
        token1 = stack;
        if (zpcisoper(token1)) {
            token1 = zpcpoptoken(&stack);
            zpcqueuetoken(token1, &queue, &tail);
        } else if ((token1)
                   && (token1->type == ZPCLEFTPAREN
                       || token1->type == ZPCRIGHTPAREN)) {
            fprintf(stderr, "mismatched parentheses: %s\n", token1->str);

            return NULL;
        }
    } while (stack);

    return queue;
}

struct zpctoken *
zpceval(struct zpctoken *srcqueue)
{
    struct zpctoken *token = srcqueue;
    struct zpctoken *queue = NULL;
    struct zpctoken *tail = NULL;
    struct zpctoken *stack = NULL;
    struct zpctoken *token1 = token;
    struct zpctoken *token2;
    struct zpctoken *arg1;
    struct zpctoken *arg2;
    int64_t          dest;
    zpccop_t        *func;
    long             radix;

    while (token) {
        token2 = token->next;
        if (zpcisvalue(token)) {
            zpcpushtoken(token, &stack);
        } else if (zpcisoper(token)) {
            if (zpccopnargtab[token->type] >= 1) {
                if (!token1) {
                    fprintf(stderr, "missing argument 1\n");

                    return NULL;
                }
            }
            arg2 = NULL;
            if (zpccopnargtab[token->type] == 2) {
                arg2 = zpcpoptoken(&stack);
                if (!arg2) {
                    fprintf(stderr, "missing argument 2\n");

                    return NULL;
                }
            }
            if (token1) {
                arg1 = token1;
            }
            switch (zpccopnargtab[token->type]) {
                case 2:
                    if (!arg2) {
                        fprintf(stderr, "invalid argument 2\n");

                        return NULL;
                    }
                case 1:
                    if (!arg1) {
                        fprintf(stderr, "invalid argument 1\n");

                        return NULL;
                    }

                    break;
            }
            func = zpcevaltab[token->type];
            if (func) {
                if (arg2) {
                    dest = func(arg2, arg1);
                    if (arg1->radix == 16 || arg2->radix == 16) {
                        token1->radix = 16;
                    } else if (arg1->radix == 8 || arg2->radix == 8) {
                        token1->radix = 8;
                    } else if (arg1->radix == 2 || arg2->radix == 2) {
                        token1->radix = 2;
                    } else {
                        token1->radix = 10;
                    }
#if (SMARTTYPES)
                    token1->type = arg1->type;
                    token1->flags = arg1->flags;
                    token1->sign = arg1->sign;
#endif
                } else {
                    dest = func(arg1, arg2);
                    token1->radix = arg1->radix;
#if (SMARTTYPES)
                    token1->type = arg2->type;
                    token1->flags = arg2->flags;
                    token1->sign = arg2->sign;
#endif
                }
                token1->data.ui64.i64 = dest;
                if (arg1->type == ZPCINT64 || arg1->type == ZPCUINT64) {
                    radix = token1->radix;
                    if (!radix) {
                        radix = zpcradix;
                    }
                    token1->radix = radix;
                    zpcprintstr64(token1, token1->data.ui64.u64, radix);
                }
            }
        }
        token = token2;
    }
    zpcqueuetoken(token1, &queue, &tail);

    return queue;
}

