#include <stdint.h>
#include <stddef.h>
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
struct zpctoken *zpcvarhash[NHASHITEM];
uint8_t          zpcdectab[256];
uint8_t          zpchextab[256];
uint8_t          zpcocttab[256];
float            zpcdecflttab[256];
float            zpchexflttab[256];
float            zpcoctflttab[256];
double           zpcdecdbltab[256];
double           zpchexdbltab[256];
double           zpcoctdbltab[256];

void
zpcinittab(void)
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
zpcgetfloat(struct zpctoken *token, const char *str)
{
    long  sign = 0;
    float flt = 0.0;
    float div;

    if (*str == '-') {
        sign = 1;
        str++;
    }
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
            if (sign) {
                zpcsetvalf32(&token->data.f32, -flt);
            } else {
                zpcsetvalf32(&token->data.f32, flt);
            }
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
            if (sign) {
                zpcsetvalf32(&token->data.f32, -flt);
            } else {
                zpcsetvalf32(&token->data.f32, flt);
            }
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
            if (sign) {
                zpcsetvalf32(&token->data.f32, -flt);
            } else {
                zpcsetvalf32(&token->data.f32, flt);
            }
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
        if (sign) {
            zpcsetvalf32(&token->data.f32, -flt);
        } else {
            zpcsetvalf32(&token->data.f32, flt);
        }
    }

    return;
}

void
zpcgetdouble(struct zpctoken *token, const char *str)
{
    long  sign = 0;
    float dbl = 0.0;
    float div;

    if (*str == '-') {
        sign = 1;
        str++;
    }
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
            if (sign) {
                zpcsetvalf64(&token->data.f64, -dbl);
            } else {
                zpcsetvalf64(&token->data.f64, dbl);
            }
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
            if (sign) {
                zpcsetvalf64(&token->data.f64, -dbl);
            } else {
                zpcsetvalf64(&token->data.f64, dbl);
            }
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
            if (sign) {
                zpcsetvalf64(&token->data.f64, -dbl);
            } else {
                zpcsetvalf64(&token->data.f64, dbl);
            }
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
        if (sign) {
            zpcsetvalf64(&token->data.f64, -dbl);
        } else {
            zpcsetvalf64(&token->data.f64, dbl);
        }
    }

    return;
}

void
zpcgetint64(struct zpctoken *token, const char *str)
{
    long    sign = 0;
    int64_t i64 = 0;

    if (*str == '-') {
        sign = 1;
        str++;
    }
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
            if (sign) {
                zpcsetval64(&token->data.i64, -i64);
            } else {
                zpcsetvalu64(&token->data.i64, i64);
            }
        } else if (*str == 'b' || *str == 'B') {
            /* binary value */
            str++;
            while (isxdigit(*str)) {
                i64 <<= 1;
                i64 += tobin(*str);
                str++;
            }
            if (sign) {
                zpcsetval64(&token->data.i64, -i64);
            } else {
                zpcsetvalu64(&token->data.i64, i64);
            }
        } else {
            /* octal value */
            str++;
            while (isxdigit(*str)) {
                i64 <<= 3;
                i64 += tooct(*str);
                str++;
            }
            if (sign) {
                zpcsetval64(&token->data.i64, -i64);
            } else {
                zpcsetvalu64(&token->data.i64, i64);
            }
        }
    } else {
        /* decimal value */
        str++;
        while (isxdigit(*str)) {
            i64 *= 10;
            i64 += todec(*str);
            str++;
        }
        if (sign) {
            zpcsetval64(&token->data.i64, -i64);
        } else {
            zpcsetvalu64(&token->data.i64, i64);
        }
    }

    return;
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
zpcgettoken(const char *str)
{
    struct zpctoken *token = NULL;
    char            *dec;
    char            *flt;

    while (isspace(*str)) {
        str++;
    }
    if (isdigit(*str) || *str == '-') {
        token = malloc(sizeof(struct zpctoken));
        token->str = NULL;
        dec = index(str, '.');
        if (dec) {
            flt = strstr(dec, "fF");
            if (flt) {
                token->type = ZPCFLOAT;
                zpcgetfloat(token, str);
            } else {
                token->type = ZPCDOUBLE;
                zpcgetdouble(token, str);
            }
        } else {
            token->type = ZPCINT64;
            zpcgetint64(token, str);
        }
    }

    return token;
}

/*
 * Dijkstra's shunting yard algorithm
 * - turns infix-format expressions into RPN and/or parse trees
 */

