#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <zpc/zpc.h>

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

#define OPERRTOL  0x80000000
#define zpccopprec(tp)                                                 \
    (zpccopprectab[(tp)->type] & ~OPERRTOL)
#define zpccopisrtol(tp)                                               \
    (zpccopprectab[(tp)->type] & OPERRTOL)
#define zpciscopchar(c) (zpccopchartab[(int)(c)])
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
static long             zpccopnargtab[ZPCNOPER];
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
    zpccopprectab[ZPCNOT] = OPERRTOL | 7;
    zpccopprectab[ZPCINC] = 7;
    zpccopprectab[ZPCDEC] = 7;
    zpccopprectab[ZPCSHR] = 6;
    zpccopprectab[ZPCSHL] = 6;
    zpccopprectab[ZPCAND] = 5;
    zpccopprectab[ZPCXOR] = OPERRTOL | 4;
    zpccopprectab[ZPCOR] = 3;
    zpccopprectab[ZPCMUL] = 2;
    zpccopprectab[ZPCDIV] = 2;
    zpccopprectab[ZPCMOD] = 2;
    zpccopprectab[ZPCADD] = OPERRTOL | 1;
    zpccopprectab[ZPCSUB] = 1;
    /* # of arguments */
    zpccopnargtab[ZPCNOT] = 1;
    zpccopnargtab[ZPCINC] = 1;
    zpccopnargtab[ZPCDEC] = 1;
    zpccopnargtab[ZPCSHR] = 2;
    zpccopnargtab[ZPCSHL] = 2;
    zpccopnargtab[ZPCAND] = 2;
    zpccopnargtab[ZPCXOR] = 2;
    zpccopnargtab[ZPCOR] = 2;
    zpccopnargtab[ZPCMUL] = 2;
    zpccopnargtab[ZPCDIV] = 2;
    zpccopnargtab[ZPCMOD] = 2;
    zpccopnargtab[ZPCADD] = 2;
    zpccopnargtab[ZPCSUB] = 2;

    return;
}

void zpcinitcop(void)
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
exprinit(void)
{
    zpcinitconvtab();
    zpcinitcoptab();
    zpcinitcop();

    return;
}

void
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
        token->prev = zpctokenqueue;
        zpctokenqueue->next = token;
        zpctokentail = token;
    }

    return;
}

void
zpcpushoper(struct zpctoken *token, struct zpctoken **stack)
{
    token->prev = NULL;
    token->next = *stack;
    *stack = token;

    return;
}

struct zpctoken *
zpcpopoper(struct zpctoken **stack)
{
    struct zpctoken *token = *stack;

    if (token) {
        *stack = token->next;
    }

    return token;
}

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
            zpcsetvalf32(&token->data.f32, flt);
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
            zpcsetvalf32(&token->data.f32, flt);
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
            zpcsetvalf32(&token->data.f32, flt);
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
        zpcsetvalf32(&token->data.f32, flt);
    }
    if (*ptr == 'f' || *ptr == 'F') {
        ptr++;
    }
    *retstr = (char *)ptr;

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
            zpcsetvalu64(&token->data.i64, i64);
        } else if (*ptr == 'b' || *ptr == 'B') {
            /* binary value */
            ptr++;
            while (isxdigit(*ptr)) {
                i64 <<= 1;
                i64 += tobin(*ptr);
                ptr++;
            }
            zpcsetvalu64(&token->data.i64, i64);
        } else {
            /* octal value */
            ptr++;
            while (isxdigit(*ptr)) {
                i64 <<= 3;
                i64 += tooct(*ptr);
                ptr++;
            }
            zpcsetvalu64(&token->data.i64, i64);
        }
    } else {
        /* decimal value */
        while (isxdigit(*ptr)) {
            i64 *= 10;
            i64 += todec(*ptr);
            ptr++;
        }
        zpcsetvalu64(&token->data.i64, i64);
    }
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
            zpcsetvalu64(&token->data.u64, u64);
        } else if (*ptr == 'b' || *ptr == 'B') {
            /* binary value */
            ptr++;
            while (isxdigit(*ptr)) {
                u64 <<= 1;
                u64 += tobin(*ptr);
                ptr++;
            }
            zpcsetvalu64(&token->data.u64, u64);
        } else {
            /* octal value */
            ptr++;
            while (isxdigit(*ptr)) {
                u64 <<= 3;
                u64 += tooct(*ptr);
                ptr++;
            }
            zpcsetvalu64(&token->data.u64, u64);
        }
    } else {
        /* decimal value */
        while (isxdigit(*ptr)) {
            u64 *= 10;
            u64 += todec(*ptr);
            ptr++;
        }
        zpcsetvalu64(&token->data.u64, u64);
    }
    *retstr = (char *)ptr;

    return;
}

void
zpcgetoper(struct zpctoken *token, const char *str, char **retstr)
{
    char *ptr = (char *)str;

    token->str = malloc(TOKENSTRLEN);
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
        ptr += 2;
        token->type = ZPCSHR;
        token->str[0] = token->str[1] = '>';
        token->str[2] = '\0';
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
    char            *flt;
    char            *unsign;

    if (!*str) {

        return NULL;
    }
    while (isspace(*ptr)) {
        ptr++;
    }
    token = malloc(sizeof(struct zpctoken));
    if (isdigit(*ptr)) {
        token->str = NULL;
        dec = index(ptr, '.');
        unsign = strstr(ptr, "uU");
        if (dec) {
            flt = strstr(dec, "fF");
            if (flt) {
                token->type = ZPCFLOAT;
                zpcgetfloat(token, ptr, &ptr);
            } else {
                token->type = ZPCDOUBLE;
                zpcgetdouble(token, ptr, &ptr);
            }
        } else if (unsign) {
            token->type = ZPCUINT64;
            zpcgetuint64(token, ptr, &ptr);
        } else {
            token->type = ZPCINT64;
            zpcgetint64(token, ptr, &ptr);
        }
    } else if (zpciscopchar(*ptr)) {
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
        *retstr = (char *)ptr;
    }

    return token;
}

void
printtoken(struct zpctoken *token)
{
    fprintf(stderr, "TOKEN of type %lx: ", token->type);
    if (zpcisoper(token)) {
        fprintf(stderr, "%s\n", token->str);
    } else if (token->type == ZPCINT64) {
        fprintf(stderr, "%llx\n", token->data.i64);
    } else if (token->type == ZPCUINT64) {
        fprintf(stderr, "%llx\n", token->data.u64);
    } else if (token->type == ZPCFLOAT) {
        fprintf(stderr, "%f\n", token->data.f32);
    } else if (token->type == ZPCDOUBLE) {
        fprintf(stderr, "%e\n", token->data.f64);
    } else if (token->type == ZPCLEFT) {
        fprintf(stderr, "(\n");
    } else if (token->type == ZPCRIGHT) {
        fprintf(stderr, ")\n");
    } else {
        fprintf(stderr, "\n");
    }
}

void
zpcprintqueue(struct zpctoken *queue)
{
    struct zpctoken *token = queue;

    fprintf(stderr, "QUEUE:\n");
    while (token) {
        printtoken(token);
        token = token->next;
    }

    return;
}

void
zpctokenize(const char *str)
{
    char            *ptr = (char *)str;
    struct zpctoken *token;

    token = zpcgettoken(ptr, &ptr);
    while (token) {
        zpcqueuetoken(token);
        token = zpcgettoken(ptr, &ptr);
    }
#if 0
    fprintf(stderr, "TOKENS: ");
    zpcprintqueue(zpctokenqueue);
#endif

    return;
};

/*
 * Dijkstra's shunting yard algorithm
 * - turns infix-format expressions into RPN queues
 * - TODO: fix to work a'la
 *    https://en.wikipedia.org/wiki/Shunting-yard_algorithm
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
        printtoken(token1);
        token = malloc(sizeof(struct zpctoken));
        token3 = token1->next;
        memcpy(token, token1, sizeof(struct zpctoken));
        token = token1;
        if (zpcisvalue(token)) {
            zpcqueueexpr(token, &queue, &tail);
        } else if (zpcisfunc(token)) {
            zpcpushoper(token, &stack);
        } else if (zpcissep(token)) {
            token2 = stack;
            while ((token2) && token2->type != ZPCLEFT) {
                token2 = zpcpopoper(&stack);
                zpcqueueexpr(token2, &queue, &tail);
                token2 = zpcpopoper(&stack);
            }
            if ((token2) && token2->type == ZPCLEFT) {
                
                continue;
            } else {
                fprintf(stderr, "mismatched parentheses: %s\n", token2->str);
                
                return 0;
            }
        } else if (zpcisoper(token)) {
            token2 = stack;
            while (zpcisoper(token2)) {
                if ((!zpccopisrtol(token)
                     && zpccopprec(token) <= zpccopprec(token2))
                    || zpccopprec(token) < zpccopprec(token2)) {
//                    fprintf(stderr, "POP: %s (%s)\n", token2->str, token->str);
                    token2 = zpcpopoper(&stack);
                    zpcqueueexpr(token2, &queue, &tail);
                    token2 = stack;
                } else {

                    break;
                }
            }
//            fprintf(stderr, "PUSH: %s\n", token->str);
            zpcpushoper(token, &stack);
        } else if (token->type == ZPCLEFT) {
            zpcpushoper(token, &stack);
        } else if (token->type == ZPCRIGHT) {
            token2 = stack;
            while ((token2) && token2->type != ZPCLEFT) {
                token2 = zpcpopoper(&stack);
                zpcqueueexpr(token2, &queue, &tail);
                token2 = stack;
            }
            if ((token2) && token2->type == ZPCLEFT) {
                token2 = zpcpopoper(&stack);
            } else {
                if (token2) {
                    fprintf(stderr, "mismatched parentheses: %s\n",
                            token2->str);
                }
                
                return 0;
            }
            if (zpcisfunc(stack)) {
                token2 = zpcpopoper(&stack);
                zpcqueueexpr(token2, &queue, &tail);
            }
        }
        token1 = token3;
    }
    fprintf(stderr, "QUEUE: ");
    zpcprintqueue(queue);
    fprintf(stderr, "STACK: ");
    zpcprintqueue(stack);
    do {
        token1 = stack;
        if (zpcisoper(token1)) {
            token1 = zpcpopoper(&stack);
            zpcqueueexpr(token1, &queue, &tail);
        } else if ((token1)
                   && (token1->type == ZPCLEFT || token1->type == ZPCRIGHT)) {
            fprintf(stderr, "mismatched parentheses: %s\n", token1->str);

            return 0;
        }
    } while (stack);

    return queue;
}


