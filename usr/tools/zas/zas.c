/* zero assembler main file */

#include <zas/conf.h>

#ifndef ZASDEBUG
#define ZASDEBUG 0
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <ctype.h>
#include <string.h>
#include <unistd.h>
#include <zero/cdecl.h>
#include <zero/param.h>
#include <zero/trix.h>
#if (ZASZEROHASH)
#include <zero/hash.h>
#endif
#if (ZASPROF)
#include <zero/prof.h>
#endif
#include <zas/zas.h>
#include <zas/opt.h>
#include <zas/io.h>
#if (ZVM)
#include <zvm/zvm.h>
#include <zvm/mem.h>
#elif (WPM)
#include <wpm/wpm.h>
#endif

extern struct zasop *    asmfindop(const uint8_t *str);
    
extern zasuword_t        asmgetreg(uint8_t *str, zasword_t *retsize,
                                   uint8_t **retptr);
static uint8_t         * zasgetlabel(uint8_t *str, uint8_t **retptr);
static struct zasop    * zasgetinst(uint8_t *str, uint8_t **retptr);
static uint8_t         * zasgetsym(uint8_t *str, uint8_t **retptr);
static zasuword_t        zasgetvalue(uint8_t *str, zasword_t *retval,
                                     uint8_t **retptr);
static uint8_t           zasgetchar(uint8_t *str, uint8_t **retptr);
#if (ZASMMAP)
static struct zastoken * zasgettoken(uint8_t *str, uint8_t **retptr,
                                     struct zasmap *map);
#else
static struct zastoken * zasgettoken(uint8_t *str, uint8_t **retptr);
#endif

static struct zastoken * zasprocvalue(struct zastoken *, zasmemadr_t, zasmemadr_t *);
static struct zastoken * zasproclabel(struct zastoken *, zasmemadr_t, zasmemadr_t *);
/* asmprocinst() is machine-specific */
extern struct zastoken * asmprocinst(struct zastoken *, zasmemadr_t, zasmemadr_t *);
static struct zastoken * zasprocchar(struct zastoken *, zasmemadr_t, zasmemadr_t *);
static struct zastoken * zasprocdata(struct zastoken *, zasmemadr_t, zasmemadr_t *);
static struct zastoken * zasprocglobl(struct zastoken *, zasmemadr_t, zasmemadr_t *);
static struct zastoken * zasprocspace(struct zastoken *, zasmemadr_t, zasmemadr_t *);
static struct zastoken * zasprocorg(struct zastoken *, zasmemadr_t, zasmemadr_t *);
static struct zastoken * zasprocalign(struct zastoken *, zasmemadr_t, zasmemadr_t *);
static struct zastoken * zasprocasciz(struct zastoken *, zasmemadr_t, zasmemadr_t *);

#if 0
static struct zasop     *zasophash[ZASNHASH] ALIGNED(PAGESIZE);
#endif
static struct zassymrec *zassymhash[ZASNHASH];
static struct zasval    *zasvalhash[ZASNHASH];
static struct zaslabel  *zasglobhash[ZASNHASH];
#if (ZASDB)
struct zasline          *zaslinehash[ZASNHASH];
#endif

zastokfunc_t            *zasktokfunctab[ZASNTOKEN]
= {
    NULL,
    zasprocvalue,
    zasproclabel,
    asmprocinst,
    NULL,               // ZASTOKENREG
    NULL,               // ZASTOKENVAREG
    NULL,               // ZASTOKENVLREG
    NULL,               // ZASTOKENSYM
    zasprocchar,
    NULL,               // ZASTOKENIMMED
    NULL,               // ZASTOKENINDIR
    NULL,               // ZASTOKENADR
    NULL,               // ZASTOKENINDEX
    zasprocdata,
    zasprocglobl,
    zasprocspace,
    zasprocorg,
    zasprocalign,
    zasprocasciz,
    NULL,
    NULL
};
#if (ZASALIGN)
zasmemadr_t              zastokalntab[ZASNTOKEN];
#endif

struct zastoken         *zastokenqueue;
struct zastoken         *zastokentail;
static struct zassymrec *symqueue;
zasmemadr_t              _startadr;
static zasmemadr_t       _startset;
unsigned long            zasinputread;
uint8_t                 *zaslinebuf;
uint8_t                 *zasstrbuf;
long                     zasnreadbuf = 16;
long                     zasreadbufcur = 0;

void
zasprinttoken(struct zastoken *token)
{
    switch (token->type) {
        case ZASTOKENVALUE:
            fprintf(stderr, "value 0x%08lx (size == %ld)\n",
                    (unsigned long) token->data.value.val, (unsigned long)token->data.value.size);
            
            break;
        case ZASTOKENLABEL:
            fprintf(stderr, "label %s (adr == 0x%08lx)\n",
                    token->data.label.name,
                    (unsigned long)token->data.label.adr);

            break;
        case ZASTOKENINST:
            fprintf(stderr, "instruction %s (op == 0x%02x)\n",
                    token->data.inst.name, token->data.inst.op);

            break;
        case ZASTOKENREG:
            fprintf(stderr, "register r%1lx\n", (long)token->data.ndx.reg);

            break;
#if (ZASVEC)
            /* vector address register */
        case ZASTOKENVAREG:
            fprintf(stderr, "address register va%1lx\n", (long)token->data.ndx.reg);

            break;
            /* vector length register */
        case ZASTOKENVLREG:
            fprintf(stderr, "length register vl%1lx\n", (long)token->data.ndx.reg);

            break;
#endif
        case ZASTOKENSYM:
            fprintf(stderr, "symbol %s (adr == 0x%08lx)\n",
                    token->data.sym.name, (long)token->data.sym.adr);

            break;
        case ZASTOKENCHAR:
            fprintf(stderr, "character 0x%02x\n", token->data.ch);

            break;
        case ZASTOKENIMMED:
            fprintf(stderr, "immediate (val == 0x%08lx)\n", (long)token->val);

            break;
        case ZASTOKENINDIR:
            fprintf(stderr, "indirection\n");

            break;
        case ZASTOKENADR:
            fprintf(stderr, "address (sym == %s, adr == 0x%08lx)\n",
                    token->data.adr.name, (long)token->data.adr.val);

            break;
        case ZASTOKENINDEX:
            fprintf(stderr, "index %ld(%%r%ld)\n", (long)token->data.ndx.val,
                    (long)token->data.ndx.reg);

            break;
    }
}

void
zasfreetoken(struct zastoken *token)
{
#if (ZASDB)
    free(token->file);
#endif
    free(token);

    return;
}

static void
zasqueuetoken(struct zastoken *token)
{
    token->next = NULL;
    if (!zastokenqueue) {
        token->prev = NULL;
        zastokenqueue = token;
    } else if (zastokentail) {
        token->prev = zastokentail;
        zastokentail->next = token;
        zastokentail = token;
    } else {
        zastokenqueue->next = token;
        token->prev = zastokenqueue;
        zastokentail = token;
    }

    return;
}

#if (ZASDB)

void
zasaddline(zasmemadr_t adr, uint8_t *data, uint8_t *filename, unsigned long line)
{
    struct zasline *newline = malloc(sizeof(struct zasline));
    unsigned long   key;

    key = (adr & 0xff) + ((adr >> 8) & 0xff) + ((adr >> 16) & 0xff) + ((adr >> 24) & 0xff);
    newline->adr = adr;
    newline->file = (uint8_t *)strdup((char *)filename);
    newline->num = line;
    newline->data = data;
    newline->next = zaslinehash[key];
    key &= (ZASNHASH - 1);
    zaslinehash[key] = newline;

    return;
}

struct zasline *
zasfindline(zasmemadr_t adr)
{
    struct zasline *line;
    unsigned long   key;

    key = (adr & 0xff) + ((adr >> 8) & 0xff) + ((adr >> 16) & 0xff) + ((adr >> 24) & 0xff);
    key &= (ZASNHASH - 1);
    line = zaslinehash[key];
    while ((line) && line->adr != adr) {
        line = line->next;
    }

    return line;
}

#endif

void
zasaddval(struct zasval *val)
{
    unsigned long   key = 0;
    uint8_t        *ptr;

    ptr = val->name;
    while (*ptr) {
        key += *ptr++;
    }
    key &= (ZASNHASH - 1);
    val->next = zasvalhash[key];
    zasvalhash[key] = val;

    return;
}

struct zasval *
zasfindval(uint8_t *str, zasword_t *valptr, uint8_t **retptr)
{
    unsigned long  key = 0;
    struct zasval *val = NULL;
    uint8_t       *ptr;
    long           len = 0;

    if ((*str) && (isalpha(*str) || *str == '_')) {
        ptr = str;
        key += *str;
        str++;
        len++;
        while ((*str) && (isalnum(*str) || *str == '_')) {
            key += *str;
            str++;
            len++;
        }
        key &= (ZASNHASH - 1);
        val = zasvalhash[key];
        while ((val) && strncmp((char *)val->name, (char *)ptr, len)) {
            val = val->next;
        }
        if (val) {
            *valptr = val->val;
            *retptr = str;
        }
    }

    return val;
}

void
zasqueuesym(struct zassymrec *sym)
{
    sym->next = NULL;
    if (!symqueue) {
        symqueue = sym;
    } else {
        sym->next = symqueue;
        symqueue = sym;
    }

    return;
}

static void
zasaddsym(struct zassymrec *sym)
{
    uint8_t       *str = sym->name;
    unsigned long  key = 0;

    while (*str) {
        key += *str++;
    }
#if (ZASZEROHASH)
    key = hashq128(&key, sizeof(unsigned long), NHASHBIT);
#else
    key &= (ZASNHASH - 1);
#endif
    sym->next = zassymhash[key];
    zassymhash[key] = sym;

    return;
}

struct zassymrec *
zasfindsym(uint8_t *name)
{
    struct zassymrec *sym = NULL;
    uint8_t          *str = name;
    unsigned long     key = 0;

    while (*str) {
        key += *str++;
    }
#if (ZASZEROHASH)
    key = hashq128(&key, sizeof(unsigned long), NHASHBIT);
#else
    key &= (ZASNHASH - 1);
#endif
    sym = zassymhash[key];
    while ((sym) && strcmp((char *)sym->name, (char *)name)) {
        sym = sym->next;
    }

    return sym;
}

void
zasremovesyms(void)
{
    struct zassymrec *sym1;
    struct zassymrec *sym2;
    long              l;

    for (l = 0 ; l < ZASNHASH ; l++) {
        sym1 = zassymhash[l];
        while (sym1) {
            sym2 = sym1;
            sym1 = sym1->next;
            free(sym2);
        }
        zassymhash[l] = NULL;
    }

    return;
}

static void
zasaddglob(struct zaslabel *label)
{
    uint8_t       *str = label->name;
    unsigned long  key = 0;

    while (*str) {
        key += *str++;
    }
    key &= (ZASNHASH - 1);
    label->next = zasglobhash[key];
    zasglobhash[key] = label;

    return;
}

struct zaslabel *
zasfindglob(uint8_t *name)
{
    struct zaslabel *label = NULL;
    uint8_t         *str = name;
    unsigned long    key = 0;

    while (*str) {
        key += *str++;
    }
    key &= (ZASNHASH - 1);
    label = zasglobhash[key];
    while ((label) && strcmp((char *)label->name, (char *)name)) {
        label = label->next;
    }

    return label;
}

static uint8_t *
zasgetlabel(uint8_t *str, uint8_t **retptr)
{
    uint8_t *ptr = str;
    uint8_t *name = NULL;

#if (ZASDEBUG)
    fprintf(stderr, "getlabel: %s\n", str);
#endif
    if ((*str) && (isalpha(*str) || *str == '_')) {
        str++;
        while ((*str) && (isalnum(*str) || *str == '_')) {
            str++;
        }
    }
    if (*str == ':') {
        *str++ = '\0';
        name = (uint8_t *)strdup((char *)ptr);
        *retptr = str;
    }

    return name;
}

static struct zasop *
zasgetinst(uint8_t *str, uint8_t **retptr)
{
    struct zasop *op;

    op = asmfindop(str);
#if (ZASDEBUG)
    fprintf(stderr, "getinst: %s\n", str);
#endif
    if (op) {
        str += op->len;
        *retptr = str;
    }

    return op;
}

static uint8_t *
zasgetsym(uint8_t *str, uint8_t **retptr)
{
    uint8_t *ptr = str;
    uint8_t *name = NULL;

#if (ZASDEBUG)
    fprintf(stderr, "getsym: %s\n", str);
#endif
    if (isalpha(*str) || *str == '_') {
        str++;
    }
    while (isalnum(*str) || *str == '_') {
        str++;
    }
    while ((*str) && (isspace(*str))) {
        str++;
    }
    if (*str == ',') {
        *str++ = '\0';
    }
    name = (uint8_t *)strdup((char *)ptr);
    *retptr = str;

    return name;
}

static zasuword_t
zasgetvalue(uint8_t *str, zasword_t *valret, uint8_t **retstr)
{
    long                found = 0;
    zasuword_t          uval = 0;
    zasword_t           val = 0;
    long                neg = 0;

#if (ZASDEBUG)
    fprintf(stderr, "getvalue: %s\n", str);
#endif
    if (*str == '-') {
        neg = 1;
        str++;
    }
    if (str[0] == '0' && tolower(str[1]) == 'x') {
        str += 2;
        while ((*str) && isxdigit(*str)) {
            uval <<= 4;
            uval += (isdigit(*str)
                     ? *str - '0'
                     : (islower(*str)
                        ? *str - 'a' + 10
                        : *str - 'A' + 10));
            str++;
        }
        found = 1;
    } else if (str[0] == '0' && tolower(str[1]) == 'b') {
        str += 2;
        while ((*str) && (*str == '0' || *str == '1')) {
            uval <<= 1;
            uval += *str - '0';
            str++;
        }
        found = 1;
    } else if (*str == '0') {
        str++;
        while ((*str) && isdigit(*str)) {
            if (*str > '7') {
                fprintf(stderr, "invalid number in octal constant: %s\n", str);

                exit(1);
            }
            uval <<= 3;
            uval += *str - '0';
            str++;
        }
        found = 1;
    } else if (isdigit(*str)) {
        while ((*str) && isdigit(*str)) {
            uval *= 10;
            uval += *str - '0';
            str++;
        }
        found = 1;
    }
    if (found) {
        *retstr = str;
        if (neg) {
            val = -((zasword_t)uval);
            *valret = val;
        } else {
            *valret = (zasword_t)uval;
        }
    }

    return found;
}

static uint8_t *
zasgetadr(uint8_t *str, uint8_t **retptr)
{
    uint8_t *ptr = str;
    uint8_t *name = NULL;
    
#if (ZASDEBUG)
    fprintf(stderr, "getadr: %s\n", str);
#endif
    while (isalpha(*str) || *str == '_') {
        str++;
    }
    if (*str == ',') {
        *str++ = '\0';
    }
    name = (uint8_t *)strdup((char *)ptr);
    *retptr = str;

    return name;
}

static zasuword_t
zasgetindex(uint8_t *str, zasword_t *retndx, uint8_t **retptr)
{
    zasuword_t reg = 0xff;
    zasword_t  val = 0;
    zasword_t  ndx = 0;
    zasword_t  size = 0;
    long       neg = 0;

#if (ZASDEBUG)
    fprintf(stderr, "getindex: %s\n", str);
#endif
    if (*str == '-') {
        neg = 1;
        str++;
    }
    if (str[0] == '0' && (str[1] == 'x' || str[1] == 'X')) {
        str += 2;
        while ((*str) && isxdigit(*str)) {
            val <<= 4;
            val += (isdigit(*str)
                    ? *str - '0'
                    : tolower(*str) - 'a' + 10);
            str++;
        }
    } else if (str[0] == '0' && (str[1] == 'b' || str[1] == 'B')) {
        str += 2;
        while ((*str) && (*str == '0' || *str == '1')) {
            val <<= 1;
            val += *str - '0';
            str++;
        }
    } else if (*str == '0') {
        str++;
        while ((*str) && isdigit(*str)) {
            if (*str > '7') {
                fprintf(stderr, "invalid number in octal constant: %s\n", str);

                exit(1);
            }
            val <<= 3;
            val += *str - '0';
            str++;
        }
    } else if (isdigit(*str)) {
        while ((*str) && isdigit(*str)) {
            val *= 10;
            val += *str - '0';
            str++;
        }
    }
    if (*str == '(') {
        ndx = 1;
        str++;
    }
    if (ndx) {
        if (*str == '%') {
            str++;
        }
        reg = asmgetreg(str, &size, &str);
#if 0
        if (reg >= ZASNREG) {
            fprintf(stderr, "invalid register name %s\n", str);

            exit(1);
        }
#endif
        *retptr = str;
    }
#if (ZASNEWHACKS)
    if (size) {
        val &= (1L << (CHAR_BIT * size)) - 1;
    }
#endif
    if (neg) {
        val = -val;
    }
    *retndx = val;

    return reg;
}

static uint8_t
zasgetchar(uint8_t *str, uint8_t **retptr)
{
    uint8_t   *name = str;
    zasword_t  val = 0;

#if (ZASDEBUG)
    fprintf(stderr,"getchar: %s\n", str);
#endif
    str++;
    if (*str == '\\') {
        str++;
        if (isalpha(*str)) {
            switch (*str) {
                case 'n':
                    val = '\n';
                    str++;

                    break;
                case 't':
                    val = '\t';
                    str++;

                    break;
                case 'r':
                    val = '\r';
                    str++;

                    break;
                default:
                    fprintf(stderr, "invalid character literal: %s\n", name);
            }
        } else {
            if (*str == '0') {
                str++;
            }
            while ((*str) && isdigit(*str)) {
                if (*str > '7') {
                    fprintf(stderr, "invalid number in octal constant: %s\n",
                            name);
                    
                    exit(1);
                }
                val <<= 3;
                val += *str - '0';
                str++;
            }
        }
    } else if (isalpha(*str) || isspace(*str)) {
        val = *str;
        str++;
    } else {
        while ((*str) && isdigit(*str)) {
            val *= 10;
            val += *str - '0';
            str++;
        }
    }
    if (val > 0xff) {
        fprintf(stderr, "oversize character literal %s (%lx)\n",
                name, (long)val);

        exit(1);
    }
    if (*str == '\'') {
        str++;
    }
    if (*str == ',') {
        str++;
    }
    *retptr = str;

    return (uint8_t)val;
}

#if (ZASMMAP)
static struct zastoken *
zasgettoken(uint8_t *str, uint8_t **retptr, struct zasmap *map)
#else
static struct zastoken *
zasgettoken(uint8_t *str, uint8_t **retptr)
#endif
{
    long             buflen = ZASLINELEN;
    long             len;
    uint8_t         *buf = zasstrbuf;
    struct zastoken *token1 = malloc(sizeof(struct zastoken));
    struct zastoken *token2;
    struct zasop    *op = NULL;
    uint8_t         *name = str;
    zasword_t        val = ZASRESOLVE;
    zasword_t        size = 0;
    zasword_t        ndx;
    int              ch;
#if (ZASDB)
    uint8_t         *ptr;
#endif

    while (*str && isspace(*str)) {
        str++;
    }
    if (*str == ',') {
        str++;
    }
    while (*str && isspace(*str)) {
        str++;
    }
#if (ZASDEBUG)
    fprintf(stderr, "gettoken: %s\n", str);
#endif
    if ((*str) && (isdigit(*str) || *str == '-')) {
        val = zasgetindex(str, &ndx, &str);
        if (val < 0xff) {
            token1->type = ZASTOKENINDEX;
            token1->data.ndx.reg = ZASREGINDEX | val;
            token1->data.ndx.val = ndx;
        } else if (zasgetvalue(str, &val, &str)) {
            token1->type = ZASTOKENVALUE;
            token1->data.value.val = val;
            token1->data.value.size = size;
        } else {
            fprintf(stderr, "invalid token %s\n", zaslinebuf);
                
            exit(1);
        }
    } else if ((*str) && *str == '"') {
        str++;
        len = 0;
        while (*str) {
            *buf++ = *str++;
            len++;
            if (len == buflen) {
                fprintf(stderr, "overlong line (%ld == %ld): %s\n",
                        len, buflen, zaslinebuf);

                exit(1);
            }
        }
        while (*str != '"') {
            ch = *str++;
            if (ch == '\\') {
                switch (*str) {
                    case 'n':
                        *buf++ = (uint8_t)'\n';
                        str++;

                        break;
                    case 'r':
                        *buf++ = (uint8_t)'\r';
                        str++;
                        
                        break;
                    case 't':
                        *buf++ = (uint8_t)'\t';
                        str++;
                        
                        break;
                }
            } else {
                *buf++ = (uint8_t)ch;
            }
            len--;
        }
        *buf++ = '\0';
        if (*str == '"') {
            str++;
            token1->type = ZASTOKENSTRING;
            token1->data.str = (uint8_t *)strdup((char *)zasstrbuf);
        }
    } else if ((*str) && *str == '%') {
        str++;
        val = asmgetreg(str, &size, &str);
#if (ZASVEC)
        if (val & ZASREGVA) {
            token1->type = ZASTOKENVAREG;
        } else if (val & ZASREGVL) {
            token1->type = ZASTOKENVLREG;
        } else {
            token1->type = ZASTOKENREG;
        }
        val &= ZASREGMASK;    // FIXME: maximum of 256 registers */
#else
        token1->type = ZASTOKENREG;
#endif
#if (ZASNEWHACKS)
        token1->size = size;
#endif
        token1->data.reg = val;
    } else if ((*str) && (isalpha(*str) || *str == '_')) {
        name = zasgetlabel(str, &str);
        if (name) {
            token1->type = ZASTOKENLABEL;
            token1->data.label.name = name;
            token1->data.label.adr = ZASRESOLVE;
        } else {
#if (ZASDB)
            ptr = str;
#endif
#if (ZASVEC)
            if (*str == 'v') {
                op = zasgetvecinst(str, &str);
                if (op) {
                    token1->unit = UNIT_VEC;
                }
            }
            if (!op) {
                op = zasgetinst(str, &str);
#if (!ZVM)
                if (op) {
                    token1->unit = UNIT_ALU;
                }
#endif
            }
#else
            op = zasgetinst(str, &str);
#endif
            if (op) {
                token1->type = ZASTOKENINST;
#if (!ZASVEC) && !ZEN && !ZVM
                token1->unit = UNIT_ALU;
#endif
                token1->data.inst.name = op->name;
                token1->data.inst.op = op->code;
                token1->data.inst.narg = op->narg;
#if (ZASDB)
                token1->data.inst.data = (uint8_t *)strdup((char *)ptr);
#endif
#if (ZASVEC)
                token1->data.inst.flg = op->flg;
#endif
            } else {
                name = zasgetsym(str, &str);
                if (name) {
                    token1->type = ZASTOKENSYM;
                    token1->data.sym.name = name;
                    token1->data.sym.adr = ZASRESOLVE;
                }
                if (!name) {
                    name = zasgetadr(str, &str);
                    if (name) {
                        token1->type = ZASTOKENSYM;
                        token1->data.sym.name = name;
                        token1->data.sym.adr = ZASRESOLVE;
                    } else {
                        fprintf(stderr, "invalid token %s\n", zaslinebuf);
                        
                        exit(1);
                    }
                }
            }
        }
    } else if ((*str) && *str == '$') {
        str++;
        if ((*str) && (isalpha(*str) || *str == '_' || *str == '-')) {
            if (zasfindval(str, &val, &str)) {
                token1->type = ZASTOKENIMMED;
                token1->val = val;
            } else if (zasgetvalue(str, &val, &str)) {
                token1->type = ZASTOKENIMMED;
                token1->val = val;
            } else {
                name = zasgetsym(str, &str);
                if (name) {
                    token1->type = ZASTOKENADR;
                    token1->data.adr.name = name;
                    token1->data.adr.val = ZASRESOLVE;
                } else {
                    fprintf(stderr, "invalid token %s\n", zaslinebuf);
                    
                    exit(1);
                }
            }
        } else if ((*str) && isdigit(*str)) {
            if (zasgetvalue(str, &val, &str)) {
                token1->type = ZASTOKENIMMED;
                token1->val = val;
            } else {
                fprintf(stderr, "invalid immediate %s\n", str);

                exit(1);
            }
        }
    } else if (*str == '\'') {
        val = zasgetchar(str, &str);
        token1->type = ZASTOKENCHAR;
        token1->data.ch = val;
    } else if (*str == '.') {
        str++;
        size = 0;
        if (!strncmp((char *)str, "quad", 4)) {
            str += 4;
            token1->type = ZASTOKENDATA;
            size = token1->data.size = 8;
        } else if (!strncmp((char *)str, "long", 4)) {
            str += 4;
            token1->type = ZASTOKENDATA;
            size = token1->data.size = 4;
        } else if (!strncmp((char *)str, "byte", 4)) {
            str += 4;
            token1->type = ZASTOKENDATA;
            size = token1->data.size = 1;
        } else if (!strncmp((char *)str, "short", 5)) {
            str += 5;
            token1->type = ZASTOKENDATA;
            size = token1->data.size = 2;
        } else if (!strncmp((char *)str, "globl", 5)) {
            str += 5;
            token1->type = ZASTOKENGLOBL;
        } else if (!strncmp((char *)str, "space", 5)) {
            str += 5;
            token1->type = ZASTOKENSPACE;
        } else if (!strncmp((char *)str, "org", 3)) {
            str += 3;
            token1->type = ZASTOKENORG;
        } else if (!strncmp((char *)str, "align", 5)) {
            str += 5;
            token1->type = ZASTOKENALIGN;
        } else if (!strncmp((char *)str, "asciz", 5)) {
            str += 5;
            token1->type = ZASTOKENASCIZ;
        }
    } else if (*str == '*' || *str == '(') {
        str++;
        token1->type = ZASTOKENINDIR;
        token2 = malloc(sizeof(struct zastoken));
        if (*str == '%') {
            str++;
            val = asmgetreg(str, &size, &str);
#if (ZASVEC)
            if (val & ZASREGVA) {
                token2->type = ZASTOKENVAREG;
            } else if (val & ZASREGVL) {
                token2->type = ZASTOKENVLREG;
            } else {
                token2->type = ZASTOKENREG;
            }
            val &= ZASREGMASK;
#else
            token2->type = ZASTOKENREG;
#endif
#if (ZASNEWHACKS)
            token2->size = size;
#endif
            token2->data.reg = ZASREGINDIR | val;
            zasqueuetoken(token1);
            token1 = token2;
        } else if (isalpha(*str) || *str == '_') {
            name = zasgetsym(str, &str);
            if (name) {
                token2->type = ZASTOKENSYM;
                token2->data.sym.name = name;
                zasqueuetoken(token1);
                token1 = token2;
            } else {
                fprintf(stderr, "invalid token %s\n", zaslinebuf);

                exit(1);
            }
        } else {
            fprintf(stderr, "invalid token %s\n", zaslinebuf);
            
            exit(1);
        }
    }
    *retptr = str;

    return token1;
}

static struct zastoken *
zasprocvalue(struct zastoken *token, zasmemadr_t adr,
             zasmemadr_t *retadr)
{
    zasmemadr_t      ret = adr + token->data.value.size;
    uint8_t         *valptr = zvmadrtoptr(adr);
    struct zastoken *retval;

    switch (token->data.value.size) {
        case 1:
            *valptr = token->data.value.val;

            break;
        case 2:
            *((uint16_t *)valptr) = token->data.value.val;

            break;
        case 4:
            *((uint32_t *)valptr) = token->data.value.val;

            break;
        case 8:
            *((uint64_t *)valptr) = token->data.value.val;

            break;
    }
    *retadr = ret;
    retval = token->next;
    zasfreetoken(token);

    return retval;
}

static struct zastoken *
zasproclabel(struct zastoken *token, zasmemadr_t adr,
             zasmemadr_t *retadr)
{
    struct zassymrec *sym;
    struct zastoken  *retval;

    if (!_startset && !strncmp((char *)token->data.label.name, "_start", 6)) {
#if (WPMTRACE)
        fprintf(stderr, "_start == 0x%08x\n", adr);
#endif
        if (adr & (sizeof(struct zvmopcode) - 1)) {
            adr = rounduppow2(adr, sizeof(struct zvmopcode));
        }
        _startadr = adr;
        _startset = 1;
    }
    sym = zasfindsym(token->data.label.name);
    if (sym) {
        sym->adr = adr;
    } else {
        sym = malloc(sizeof(struct zassymrec));
        sym->name = token->data.label.name;
        sym->adr = adr;
        zasaddsym(sym);
    }
    *retadr = adr;
    retval = token->next;
    zasfreetoken(token);

    return retval;
}

static struct zastoken *
zasprocchar(struct zastoken *token, zasmemadr_t adr,
            zasmemadr_t *retadr)
{
    uint8_t         *valptr = zvmadrtoptr(adr);
    struct zastoken *retval;
    
    *valptr = token->data.ch;
    adr++;
    *retadr = adr;
    retval = token->next;
    zasfreetoken(token);

    return retval;
}

static struct zastoken *
zasprocdata(struct zastoken *token, zasmemadr_t adr,
            zasmemadr_t *retadr)
{
    struct zastoken *token1 = token->next;
    zasmemadr_t      valadr = adr;

    while ((token1) && token1->type == ZASTOKENVALUE) {
        token1->data.value.size = token->data.size;
        token1 = zasprocvalue(token1, valadr, &valadr);
    }
    *retadr = valadr;

    return token1;
}

static struct zastoken *
zasprocglobl(struct zastoken *token, zasmemadr_t adr,
             zasmemadr_t *retadr)
{
    struct zastoken *token1;
    struct zaslabel *label;

    token1 = token->next;
    while ((token1) && token1->type == ZASTOKENSYM) {
        token1 = zasproclabel(token1, adr, &adr);
        label = malloc(sizeof(label));
        label->name = (uint8_t *)strdup((char *)token1->data.label.name);
        label->adr = adr;
        zasaddglob(label);
    }
    *retadr = adr;

    return token1;
}

static struct zastoken *
zasprocspace(struct zastoken *token, zasmemadr_t adr,
             zasmemadr_t *retadr)
{
    struct zastoken *token1;
    struct zastoken *token2;
    zasmemadr_t      spcadr;
    uint8_t         *ptr;
    uint8_t          val;

    token1 = token->next;
    if ((token1) && token1->type == ZASTOKENVALUE) {
        spcadr = token1->data.value.val;
        token2 = token1->next;
        if ((token2) && token2->type == ZASTOKENVALUE) {
            ptr = zvmadrtoptr(spcadr);
            val = token2->data.value.val;
            while (adr < spcadr) {
                ptr[0] = val;
                adr++;
                ptr++;
            }
            token1 = token2->next;
        } else {
            adr = spcadr;
        }
    } else {
        fprintf(stderr, "invalid .space attribute token type %lx\n",
                token1->type);
        
        exit(1);
    }
    *retadr = adr;
    
    return token1;
}

static struct zastoken *
zasprocorg(struct zastoken *token, zasmemadr_t adr,
           zasmemadr_t *retadr)
{
    struct zastoken *token1;
    zasmemadr_t      orgadr;
    uint8_t         *ptr;
    uint8_t          val;
    
    token1 = token->next;
    if ((token1) && token1->type == ZASTOKENVALUE) {
        ptr = zvmadrtoptr(adr);
        orgadr = token1->data.value.val;
        val = token1->data.value.val;
        while (adr < orgadr) {
            *ptr++ = val;
            adr++;
        }
        *retadr = adr;
    }
    
    return token1;
}

static struct zastoken *
zasprocalign(struct zastoken *token, zasmemadr_t adr,
             zasmemadr_t *retadr)
{
    struct zastoken *token1;

    token1 = token->next;
    if ((token1) && token1->type == ZASTOKENVALUE) {
        adr = rounduppow2(adr, token1->data.value.val);
    } else {
        fprintf(stderr, "invalid .align attribute token type %lx\n",
                token1->type);

        exit(1);
     }
    *retadr = adr;

    return token1;
}

static struct zastoken *
zasprocasciz(struct zastoken *token, zasmemadr_t adr,
             zasmemadr_t *retadr)
{
    struct zastoken *token1;
    struct zastoken *token2;
    long             len = 0;
    uint8_t         *ptr;
    uint8_t         *str;

    token1 = token->next;
    while ((token1) && token1->type == ZASTOKENSTRING) {
        ptr = zvmadrtoptr(adr);
        str = token1->data.str;
        while ((*str) && *str != '\"') {
            if (*str == '\\') {
                str++;
                switch (*str) {
                    case 'n':
                        *ptr++ = '\n';
                        str++;
                        
                        break;
                    case 't':
                        *ptr++ = '\t';
                        str++;
                    
                        break;
                    case 'r':
                        *ptr++ = '\r';
                        str++;
                        
                        break;
                    default:
                        fprintf(stderr, "invalid character literal: \'%s\n", str);

                        exit(1);
                }
            } else {
                *ptr++ = *str++;
            }
            len++;
        }
        if (*str == '\"') {
            *ptr = '\0';
            len++;
        }
        adr += len;
        token2 = token1;
        token1 = token1->next;
        zasfreetoken(token2);
     }
    *retadr = adr;

    return token1;
}

void
zasinit(void)
{
    zasinitbuf();
#if (ZASALIGN)
    zasinitalign();
#endif
}

zasmemadr_t
zastranslate(zasmemadr_t base)
{
    zasmemadr_t      adr = base;
    struct zastoken *token = zastokenqueue;
    struct zastoken *token1;
    zastokfunc_t    *func;

    while (token) {
        func = zasktokfunctab[token->type];
        if (func) {
#if (ZASALIGN)
            adr = zasaligntok(adr, token->type);
#endif
            token1 = func(token, adr, &adr);
            if (!token) {

                break;
            }
        } else {
            fprintf(stderr, "stray token of type %lx\n", token->type);
            zasprinttoken(token);

            exit(1);
        }
        token = token1;
    }

    return adr;
}

void
zasresolve(zasmemadr_t base)
{
    struct zassymrec *sym = symqueue;
    struct zassymrec *sym1;
    struct zassymrec *item;
    struct zaslabel  *label;

    while (sym) {
        if (sym->adr == ZASRESOLVE) {
            fprintf(stderr, "unresolved symbol %s\n", sym->name);

            exit(1);
        }
        item = zasfindsym(sym->name);
        if (item) {
            if (item->adr == ZASRESOLVE) {
                fprintf(stderr, "invalid symbol %s\n", item->name);

                exit(1);
            }
            *((zasmemadr_t *)sym->adr) = item->adr;
        } else {
            label = zasfindglob(sym->name);
            if (label) {
                *((zasmemadr_t *)sym->adr) = label->adr;
            } else {
                fprintf(stderr, "unresolved symbol %s\n", sym->name);
                
                exit(1);
            }
        }
        sym1 = sym;
        sym = sym->next;
        free(sym1);
    }
    symqueue = NULL;

    return;
}

#if (ZASBUF)
void
zasreadfile(char *name, zasmemadr_t adr, int bufid)
#else
void
zasreadfile(char *name, zasmemadr_t adr)
#endif
{
#if (ZASMMAP)
    struct zasmap    map;
    struct stat      statbuf;
    int              sysret;
    int              fd;
    uint8_t         *base;
#endif
    long             buflen = ZASLINELEN;
#if (ZASBUF)
    int              fd = -1;
#else
    FILE            *fp = fopen((char *)name, "r");
#endif
    long             eof = 0;
    struct zastoken *token = NULL;
    struct zasval   *def;
    uint8_t         *fname;
    uint8_t         *ptr;
    uint8_t         *str = zaslinebuf;
    uint8_t         *lim = NULL;
    long             loop = 1;
    int              ch;
    long             comm = 0;
    long             done = 1;
    long             len = 0;
#if (ZASDB)
    unsigned long    line = 0;
#endif
    zasword_t        val = 0;
    
#if (ZASBUF) || (ZASMMAP)
    do {
        fd = open((const char *)name, O_RDONLY);
        if (fd < 0) {
            if (errno == EINTR) {
                
                continue;
            } else {
                
                break;
            }
        }
    } while (fd < 0);
#endif
#if (ZASMMAP)
    do {
        sysret = stat(name, &statbuf);
    } while (sysret < 0 && errno == EINTR);
    if (sysret < 0) {
        fprintf(stderr, "cannot stat %s\n", name);
    }
    base = mmap(NULL, statbuf.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (base == (void *)(-1)) {
        fprintf(stderr, "cannot map %s\n", name);
    }
    map.adr = base;
    map.cur = base;
    map.sz = statbuf.st_size;
    map.lim = base + map.sz;
#endif
    while (loop) {
#if (ZASMMAP)
        if (map.cur > map.adr + map.sz) {

            break;
        }
#endif
        if (done) {
            if (eof) {
                loop = 0;
//                done = 0;
                
                break;
            }
            str = zaslinebuf;
            done = 0;
#if (ZASMMAP)
            ch = zasgetc(&map);
#elif (ZASBUF)
            ch = zasgetc(fd, bufid);
#else
            ch = fgetc(fp);
#endif
            if (ch == EOF) {
                loop = 0;
                
                break;
            } else {
                len = 0;
#if (ZASDB)
                line++;
#endif
                while (ch != EOF && ch != '\n') {
                    *str++ = (uint8_t)ch;
                    len++;
                    if (len == buflen) {
                        fprintf(stderr, "overlong line (%ld == %ld): %s\n",
                                len, buflen, zaslinebuf);
                        
                        exit(1);
                    }
#if (ZASMMAP)
                    ch = zasgetc(&map);
#elif (ZASBUF)
                    ch = zasgetc(fd, bufid);
#else
                    ch = fgetc(fp);
#endif
                }
                eof = (ch == EOF);
                *str = '\0';
                str = zaslinebuf;
                lim = str + len;
                while ((*str) && isspace(*str)) {
                    str++;
                }
                if (str > lim) {
                    done = 1;
                }
            }
        } else if (!strncmp((char *)str, ".define", 7)) {
            str += 7;
            while ((*str) && isspace(*str)) {
                str++;
            }
            if ((*str) && (isalpha(*str) || *str == '_')) {
                ptr = str;
                str++;
                while ((*str) && (isalnum(*str) || *str == '_')) {
                    str++;
                }
                *str++ = '\0';
                while ((*str) && isspace(*str)) {
                    str++;
                }
                if (zasgetvalue(str, &val, &str)) {
                    def = malloc(sizeof(struct zasval));
                    def->name = (uint8_t *)strdup((char *)ptr);
                    def->val = val;
                    zasaddval(def);
                } else {
                    fprintf(stderr, "invalid .define directive %s\n", ptr);
                    
                    exit(1);
                }
            }
        } else if (!strncmp((char *)str, ".include", 8)) {
            str += 8;
            while ((*str) && isspace(*str)) {
                str++;
            }
            if (*str == '<') {
                str++;
                fname = str;
                while ((*str) && *str != '>') {
                    str++;
                }
                if (*str == '>') {
                    *str = '\0';
#if (ZASBUF)
                    zasreadfile((char *)fname, adr, ++bufid);
                    bufid--;
#else
                    zasreadfile((char *)fname, adr);
#endif
                    zasresolve(adr);
                    zasremovesyms();
                } else {
                    fprintf(stderr, "invalid .include directive %s\n",
                            str);
                    
                    exit(1);
                }
            }
            done = 1;
        } else if (!strncmp((char *)str, ".import", 7)) {
            str += 7;
            while ((*str) && isspace(*str)) {
                str++;
            }
            if (*str == '<') {
                str++;
                fname = str;
                while ((*str) && *str != '>') {
                    str++;
                }
                if (*str == '>') {
                    *str = '\0';
#if (ZASBUF)
                    zasreadfile((char *)fname, adr, ++bufid);
                    bufid--;
#else
                    zasreadfile((char *)fname, adr);
#endif
                    zasresolve(adr);
                    zasremovesyms();
                } else {
                    fprintf(stderr, "invalid .import directive %s\n",
                            zasstrbuf);

                    exit(1);
                }
            }
            done = 1;
        } else if (str[0] == ';'
                   || (str[0] == '/' && str[1] == '/')) {
            /* the rest of the line is comment */
            done = 1;
        } else if (str[0] == '/' && str[1] == '*') {
            /* comment */
            comm = 1;
            while (comm) {
#if (ZASMMAP)
                ch = zasgetc(&map);
#elif (ZASBUF)
                ch = zasgetc(fd, bufid);
#else
                ch = fgetc(fp);
#endif
                if (ch == EOF) {
                    loop = 0;

                    break;
#if (ZASDB)
                } else if (ch == '\n') {
                    line++;
#endif
                } else if (ch == '*') {
#if (ZASMMAP)
                    ch = zasgetc(&map);
#elif (ZASBUF)
                    ch = zasgetc(fd, bufid);
#else
                    ch = fgetc(fp);
#endif
                    if (ch == '/') {
                        
                        comm = 0;
                    } else if (ch == EOF) {
                        comm = 0;
                        loop = 0;
                        eof = 1;
                    }
                }
            }
            done = 1;
        } else {
            if (*str) {
#if (ZASMMAP)
                token = zasgettoken(str, &str, &map);
#else
                token = zasgettoken(str, &str);
#endif
                if (token) {
#if (ZASDB)
                    token->file = (uint8_t *)strdup((char *)name);
                    token->line = line;
#endif
                    zasqueuetoken(token);
                }
                while (isspace(*str)) {
                    str++;
                }
                if (str >= lim) {
                    done = 1;
                }
            } else {
                done = 1;
            }
        }
    }
#if (ZASBUF)
    close(fd);
#else
    fclose(fp);
#endif
#if (ZASMMAP)
    munmap(map.adr, map.sz);
#endif

    return;
}

