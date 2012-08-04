#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <zpc/zpc.h>

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

#define NHASHITEM 1024
#define OPERRTOL  0x80000000
#define zpccopprec(tp)                                                 \
    (zpccopprectab[(tp)->type] & ~OPERRTOL)
#define zpccopisrtol(tp)                                               \
    (zpccopprectab[(tp)->type] & OPERRTOL)
#define zpcisoperstr(c) (zpccopchartab[(int)(c)])
static struct zpctoken *zpcvarhash[NHASHITEM];
static uint8_t          zpcdectab[256];
static uint8_t          zpchextab[256];
static uint8_t          zpcocttab[256];
static float            zpcdecflttab[256];
static float            zpchexflttab[256];
static float            zpcoctflttab[256];
static double           zpcdecdbltab[256];
static double           zpchexdbltab[256];
static double           zpcoctdbltab[256];
static uint8_t          zpccopchartab[256];
static long             zpccopprectab[ZPCNOPER];
static struct zpctoken *zpctokenqueue;
static struct zpctoken *zpcparsequeue;
static struct zpctoken *zpctokentail;
static struct zpctoken *zpcoperstk;

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
}

void
zpcinitprectab(void)
{
    zpccopprectab[ZPCNOT] = OPERRTOL | 1;
    zpccopprectab[ZPCAND] = 7;
    zpccopprectab[ZPCOR] = 9;
    zpccopprectab[ZPCXOR] = 8;
    zpccopprectab[ZPCSHL] = 4;
    zpccopprectab[ZPCSHR] = 4;
    zpccopprectab[ZPCINC] = OPERRTOL | 1;
    zpccopprectab[ZPCDEC] = OPERRTOL | 1;
    zpccopprectab[ZPCADD] = OPERRTOL | 1;
    zpccopprectab[ZPCDEC] = OPERRTOL | 1;
    zpccopprectab[ZPCMUL] = 2;
    zpccopprectab[ZPCDIV] = 2;
    zpccopprectab[ZPCMOD] = 2;
}

void zpcinitoper(void)
{
    zpccopchartab['~'] = '~';
    zpccopchartab['&'] = '&';
    zpccopchartab['|'] = '|';
    zpccopchartab['^'] = '^';
    zpccopchartab['<'] = '<';
    zpccopchartab['>'] = '>';
    zpccopchartab['+'] = '+';
    zpccopchartab['-'] = '-';
    zpccopchartab['*'] = '*';
    zpccopchartab['/'] = '/';
    zpccopchartab['%'] = '%';
}

void
zpcqueueexpr(struct zpctoken *token)
{
    token->next = NULL;
    if (!zpcparsequeue) {
        token->prev = NULL;
        zpcparsequeue = token;
    } else if (zpctokentail) {
        token->prev = zpctokentail;
        zpctokentail->next = token;
        zpctokentail = token;
    } else {
        zpcparsequeue->next = token;
        token->prev = zpcparsequeue;
        zpctokentail = token;
    }

    return;
}

void
zpcqueuetoken(struct zpctoken *token)
{
    token->next = NULL;
    if (!zpctokenqueue) {
        token->prev = NULL;
        zpctokenqueue = token;
    } else if (zpctokentail) {
        token->prev = zpctokentail;
        zpctokentail->next = token;
        zpctokentail = token;
    } else {
        zpctokenqueue->next = token;
        token->prev = zpctokenqueue;
        zpctokentail = token;
    }

    return;
}

void
zpcpushoper(struct zpctoken *token)
{
    token->next = zpcoperstk;
    zpcoperstk = token;

    return;
}

struct zpctoken *
zpcpoptoken(void)
{
    struct zpctoken *token = zpcoperstk;

    if (token) {
        zpcoperstk = token->next;
    }

    return token;
}

void
zpcgetfloat(struct zpctoken *token, const char *str, char **retstr)
{
    float flt = 0.0;
    float div;

    if (*str == '0') {
        str++;
        if (*str == 'x' || *str == 'X') {
            /* hexadecimal value */
            str++;
            while (*str != '.' && isxdigit(*str)) {
                flt *= 16.0f;
                flt += tohexflt(*str);
                str++;
            }
            str++;
            div = 16.0f;
            while (isxdigit(*str)) {
                flt += tohexflt(*str) / div;
                div *= 16.0f;
                str++;
            }
            zpcsetvalf32(&token->data.f32, flt);
        } else if (*str == 'b' || *str == 'B') {
            /* binary value */
            str++;
            while (*str != '.' && isbdigit(*str)) {
                flt *= 2.0f;
                flt += tobinflt(*str);
                str++;
            }
            str++;
            div = 2.0f;
            while (isbdigit(*str)) {
                flt += tohexflt(*str) / div;
                div *= 2.0f;
                str++;
            }
            zpcsetvalf32(&token->data.f32, flt);
        } else {
            /* octal value */
            str++;
            while (*str != '.' && isodigit(*str)) {
                flt *= 8.0f;
                flt += tobinflt(*str);
                str++;
            }
            str++;
            div = 8.0f;
            while (isodigit(*str)) {
                flt += tohexflt(*str) / div;
                div *= 8.0f;
                str++;
            }
            zpcsetvalf32(&token->data.f32, flt);
        }
    } else {
        /* decimal value */
        while (*str != '.' && isdigit(*str)) {
            flt *= 10.0f;
            flt += tobinflt(*str);
            str++;
        }
        str++;
        div = 10.0;
        while (isdigit(*str)) {
            flt += tohexflt(*str) / div;
            div *= 10.0;
            str++;
        }
        zpcsetvalf32(&token->data.f32, flt);
    }
    *retstr = (char *)str;

    return;
}

void
zpcgetdouble(struct zpctoken *token, const char *str, char **retstr)
{
    float dbl = 0.0;
    float div;

    if (*str == '0') {
        str++;
        if (*str == 'x' || *str == 'X') {
            /* hexadecimal value */
            str++;
            while (*str != '.' && isxdigit(*str)) {
                dbl *= 16.0;
                dbl += tohexdbl(*str);
                str++;
            }
            str++;
            div = 16.0;
            while (isxdigit(*str)) {
                dbl += tohexdbl(*str) / div;
                div *= 16.0;
                str++;
            }
            zpcsetvalf64(&token->data.f64, dbl);
        } else if (*str == 'b' || *str == 'B') {
            /* binary value */
            str++;
            while (*str != '.' && isbdigit(*str)) {
                dbl *= 2.0;
                dbl += tobindbl(*str);
                str++;
            }
            str++;
            div = 2.0;
            while (isbdigit(*str)) {
                dbl += tohexdbl(*str) / div;
                div *= 2.0;
                str++;
            }
            zpcsetvalf64(&token->data.f64, dbl);
        } else {
            /* octal value */
            str++;
            while (*str != '.' && isodigit(*str)) {
                dbl *= 8.0;
                dbl += tobindbl(*str);
                str++;
            }
            str++;
            div = 8.0;
            while (isodigit(*str)) {
                dbl += tohexdbl(*str) / div;
                div *= 8.0;
                str++;
            }
            zpcsetvalf64(&token->data.f64, dbl);
        }
    } else {
        /* decimal value */
        while (*str != '.' && isdigit(*str)) {
            dbl *= 10.0;
            dbl += tobindbl(*str);
            str++;
        }
        str++;
        div = 10.0;
        while (isdigit(*str)) {
            dbl += tohexdbl(*str) / div;
            div *= 10.0;
            str++;
        }
        zpcsetvalf64(&token->data.f64, dbl);
    }
    *retstr = (char *)str;

    return;
}

void
zpcgetint64(struct zpctoken *token, const char *str, char **retstr)
{
    int64_t i64 = 0;

    if (*str == '0') {
        str++;
        if (*str == 'x' || *str == 'X') {
            /* hexadecimal value */
            str++;
            while (isxdigit(*str)) {
                i64 <<= 4;
                i64 += tohex(*str);
                str++;
            }
            zpcsetvalu64(&token->data.i64, i64);
        } else if (*str == 'b' || *str == 'B') {
            /* binary value */
            str++;
            while (isxdigit(*str)) {
                i64 <<= 1;
                i64 += tobin(*str);
                str++;
            }
            zpcsetvalu64(&token->data.i64, i64);
        } else {
            /* octal value */
            str++;
            while (isxdigit(*str)) {
                i64 <<= 3;
                i64 += tooct(*str);
                str++;
            }
            zpcsetvalu64(&token->data.i64, i64);
        }
    } else {
        /* decimal value */
        str++;
        while (isxdigit(*str)) {
            i64 *= 10;
            i64 += todec(*str);
            str++;
        }
        zpcsetvalu64(&token->data.i64, i64);
    }
    *retstr = (char *)str;

    return;
}

void
zpcgetoper(struct zpctoken *token, const char *str, char **retstr)
{
    if (*str == '!') {
        str++;
        token->type = ZPCNOT;
    } else if (*str == '&') {
        str++;
        token->type = ZPCAND;
    } else if (*str == '|') {
        str++;
        token->type = ZPCOR;
    } else if (*str == '^') {
        str++;
        token->type = ZPCXOR;
    } else if (*str == '<' && str[1] == '<') {
        str += 2;
        token->type = ZPCSHL;
    } else if (*str == '>' && str[1] == '>') {
        str += 2;
        token->type = ZPCSHR;
    } else if (*str == '+') {
        if (str[1] == '+') {
            token->type = ZPCINC;
            str += 2;
        } else {
            token->type = ZPCADD;
            str++;
        }
    } else if (*str == '-') {
        if (str[1] == '-') {
            token->type = ZPCDEC;
            str += 2;
        } else {
            token->type = ZPCSUB;
            str++;
        }
    } else if (*str == '*') {
        str++;
        token->type = ZPCMUL;
    } else if (*str == '/') {
        str++;
        token->type = ZPCDIV;
    } else if (*str == '%') {
        str++;
        token->type = ZPCMOD;
    }
    *retstr = (char *)str;
}

void
zpcaddvar(struct zpctoken *token)
{
    long  key = 0;
    char *cp;

    cp = token->str;
    while (*cp) {
        key += *cp++;
    }
    key &= NHASHITEM - 1;
    token->next = zpcvarhash[key];
    if (token->next) {
        token->next->prev = token;
    }
    zpcvarhash[key] = token;

    return;
}

struct zpctoken *
zpcfindvar(const char *name)
{
    long             key = 0;
    struct zpctoken *token = NULL;

    while (*name) {
        key += *name++;
    }
    key &= NHASHITEM - 1;
    token = zpcvarhash[key];
    while ((token) && strcmp(name, token->str)) {
        token = token->next;
    }

    return token;
}

void
zpcdelvar(const char *name)
{
    char            *cp;
    long             key = 0;
    struct zpctoken *token = NULL;

    cp = token->str;
    while (*cp) {
        key += *cp++;
    }
    key &= NHASHITEM - 1;
    token = zpcvarhash[key];
    while ((token) && strcmp(name, token->str)) {
        token = token->next;
    }
    if (token) {
        if (token->prev) {
            token->prev->next = token->next;
        } else {
            zpcvarhash[key] = token->next;
        }
        if (token->next) {
            token->next->prev = token->prev;
        }
    }

    return;
}

struct zpctoken *
zpcgettoken(const char *str, char **retstr)
{
    char            *ptr = (char *)str;
    struct zpctoken *token = NULL;
    char            *dec;
    char            *flt;

    while (isspace(*ptr)) {
        ptr++;
    }
    token = malloc(sizeof(struct zpctoken));
    if (isdigit(*ptr)) {
        token->str = NULL;
        dec = index(ptr, '.');
        if (dec) {
            flt = strstr(dec, "fF");
            if (flt) {
                token->type = ZPCFLOAT;
                zpcgetfloat(token, ptr, &ptr);
            } else {
                token->type = ZPCDOUBLE;
                zpcgetdouble(token, ptr, &ptr);
            }
        } else {
            token->type = ZPCINT64;
            zpcgetint64(token, ptr, &ptr);
        }
    } else if (zpcisoperstr(*ptr)) {
        zpcgetoper(token, ptr, &ptr);
    } else if (*ptr == '(') {
        token->type = ZPCLEFT;
        ptr++;
    } else if (*ptr == ')') {
        token->type = ZPCRIGHT;
        ptr++;
    } else {
        free(token);
        token = NULL;
    }
    if (token) {
        zpcqueuetoken(token);
        *retstr = (char *)ptr;
    } else {
        *retstr = NULL;
    }

    return token;
}

void
zpcprintqueue(struct zpctoken *queue)
{
    struct zpctoken *token = queue;

    while (token) {
        fprintf(stderr, "%lx ", token->type);
        token = token->next;
    }

    return;
}

/*
 * Dijkstra's shunting yard algorithm
 * - turns infix-format expressions into RPN queues
 */
long
zpcparse(const char *str)
{
    char            *cp = (char *)str;
    char            *ptr = (char *)str;
    struct zpctoken *token1;
    struct zpctoken *token2;
#if 0
    struct zpctoken *queue = NULL;
    struct zpctoken *tail = NULL;
#endif

    while (*ptr) {
        token1 = zpcgettoken(ptr, &ptr);
        if (token1) {
            if (zpcisvalue(token1)) {
                zpcqueueexpr(token1);
            } else if (zpcisfunc(token1)) {
                zpcpushoper(token1);
            } else if (zpcissep(token1)) {
                token1 = zpcpoptoken();
                do {
                    zpcqueueexpr(token1);
                    token1 = zpcpoptoken();
                } while ((token1) && token1->type != ZPCLEFT);
                if ((token1) && token1->type == ZPCLEFT) {

                    continue;
                } else {
                    fprintf(stderr, "mismatched parentheses: %s\n", cp);

                    return 0;
                }
            } else if (zpcisoper(token1)) {
                do {
                    token2 = zpcoperstk;
                    if (token2) {
                        if ((!zpccopisrtol(token1)
                             && zpccopprec(token1) >= zpccopprec(token2))
                            || zpccopprec(token1) > zpccopprec(token2)) {
                            token2 = zpcpoptoken();
                            zpcqueueexpr(token2);
                        }
                    }
                } while (zpcisoper(zpcoperstk));
                zpcpushoper(token1);
            } else if (token1->type == ZPCLEFT) {
                zpcpushoper(token1);
            } else if (token1->type == ZPCRIGHT) {
                do {
                    token2 = zpcoperstk;
                    if (token2) {
                        token2 = zpcpoptoken();
                        zpcqueueexpr(token2);
                    }
                } while ((zpcoperstk) && zpcoperstk->type != ZPCLEFT);
                if ((zpcoperstk) && zpcoperstk->type == ZPCLEFT) {
                    token2 = zpcpoptoken();
                } else {
                    fprintf(stderr, "mismatched parentheses: %s\n", cp);

                    return 0;
                }
                if (zpcisfunc(zpcoperstk)) {
                    token2 = zpcpoptoken();
                    zpcqueueexpr(token2);
                }
            }
        }
    }
    do {
        token1 = zpcoperstk;
        if (zpcisoper(token1)) {
            token1 = zpcpoptoken();
            zpcqueueexpr(token1);
        } else if ((token1)
                   && (token1->type == ZPCLEFT || token1->type == ZPCRIGHT)) {
            fprintf(stderr, "mismatched parentheses: %s\n", cp);

            return 0;
        }
    } while (zpcoperstk);
    zpcprintqueue(zpcparsequeue);

    return 1;
}

