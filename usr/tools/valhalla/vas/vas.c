/* valhalla assembler main file */

#include <vas/conf.h>

#ifndef VASDEBUG
#define VASDEBUG 0
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <ctype.h>
#include <string.h>
#include <unistd.h>
#include <zero/cdefs.h>
#include <zero/param.h>
#include <zero/trix.h>
#if (VASZEROHASH)
#include <zero/hash.h>
#endif
#if (VASPROF)
#include <zero/prof.h>
#endif
#include <vas/vas.h>
#include <vas/opt.h>
#include <vas/io.h>
#if (V0)
#include <v0/vm.h>
#endif

extern struct v0       * v0vm;

extern struct vasop    * vasfindop(const char *str);

extern vasuword_t        vasgetreg(char *str, vasword_t *retsize,
                                   char **retptr);
static char            * vasgetlabel(char *str, char **retptr);
static struct vasop    * vasgetinst(char *str, char **retptr);
static char            * vasgetsym(char *str, char **retptr);
static long              vasgetvalue(char *str, vasword_t *retval,
                                     char **retptr);
static char *            vasgetdef(char *srt, char **retptr);
static int               vasgetchar(char *str, char **retptr);
#if (VASMMAP)
static struct vastoken * vasgettoken(char *str, char **retptr,
                                     struct vasmap *map);
#else
static struct vastoken * vasgettoken(char *str, char **retptr);
#endif

static struct vastoken * vasprocvalue(struct vastoken *, vasmemadr_t, vasmemadr_t *);
static struct vastoken * vasproclabel(struct vastoken *, vasmemadr_t, vasmemadr_t *);
/* vasprocinst() is machine-specific */
extern struct vastoken * vasprocinst(struct vastoken *, vasmemadr_t, vasmemadr_t *);
static struct vastoken * vasprocchar(struct vastoken *, vasmemadr_t, vasmemadr_t *);
static struct vastoken * vasprocdata(struct vastoken *, vasmemadr_t, vasmemadr_t *);
static struct vastoken * vasprocglobl(struct vastoken *, vasmemadr_t, vasmemadr_t *);
static struct vastoken * vasprocspace(struct vastoken *, vasmemadr_t, vasmemadr_t *);
static struct vastoken * vasprocorg(struct vastoken *, vasmemadr_t, vasmemadr_t *);
static struct vastoken * vasprocalign(struct vastoken *, vasmemadr_t, vasmemadr_t *);
static struct vastoken * vasprocasciz(struct vastoken *, vasmemadr_t, vasmemadr_t *);

static struct vassymrec *vassymhash[VASNHASH] ALIGNED(PAGESIZE);
static struct vasdef    *vasdefhash[VASNHASH];
static struct vasval    *vasvalhash[VASNHASH];
static struct vaslabel  *vasglobhash[VASNHASH];
#if (VASDB)
struct vasline          *vaslinehash[VASNHASH];
#endif

vastokfunc_t            *vasktokfunctab[VASNTOKEN]
= {
    NULL,
    vasprocvalue,
    vasproclabel,
    vasprocinst,
    NULL,               // VASTOKENREG
    NULL,               // VASTOKENVAREG
    NULL,               // VASTOKENVLREG
    NULL,               // VASTOKENSYM
    vasprocchar,
    NULL,               // VASTOKENIMMED
    NULL,               // VASTOKENINDIR
    NULL,               // VASTOKENADR
    NULL,               // VASTOKENINDEX
    vasprocdata,
    vasprocglobl,
    vasprocspace,
    vasprocorg,
    vasprocalign,
    vasprocasciz,
    NULL,
    NULL
};
#if (VASALIGN)
vasmemadr_t              vastokalntab[VASNTOKEN];
#endif

struct vastoken         *vastokenqueue;
struct vastoken         *vastokentail;
static struct vassymrec *symqueue;
vasmemadr_t              _startadr;
static vasmemadr_t       _startset;
unsigned long            vasinputread;
char                    *vaslinebuf;
char                    *vasstrbuf;
long                     vasnreadbuf = 16;
long                     vasreadbufcur = 0;

void
vasprinttoken(struct vastoken *token)
{
    switch (token->type) {
        case VASTOKENVALUE:
            fprintf(stderr, "value 0x%08lx (size == %ld)\n",
                    (unsigned long) token->data.value.val, (unsigned long)token->data.value.size);

            break;
        case VASTOKENLABEL:
            fprintf(stderr, "label %s (adr == 0x%08lx)\n",
                    token->data.label.name,
                    (unsigned long)token->data.label.adr);

            break;
        case VASTOKENINST:
            fprintf(stderr, "instruction %s (op == 0x%02x)\n",
                    token->data.inst.name, token->data.inst.op);

            break;
        case VASTOKENREG:
            fprintf(stderr, "register r%1lx\n", (long)token->data.ndx.reg);

            break;
        case VASTOKENSYM:
            fprintf(stderr, "symbol %s (adr == 0x%08lx)\n",
                    token->data.sym.name, (long)token->data.sym.adr);

            break;
        case VASTOKENCHAR:
            fprintf(stderr, "character 0x%02x\n", token->data.ch);

            break;
        case VASTOKENIMMED:
            fprintf(stderr, "immediate (val == 0x%08lx)\n", (long)token->val);

            break;
        case VASTOKENINDIR:
            fprintf(stderr, "indirection\n");

            break;
        case VASTOKENADR:
            fprintf(stderr, "address (sym == %s, adr == 0x%08lx)\n",
                    token->data.adr.name, (long)token->data.adr.val);

            break;
        case VASTOKENINDEX:
            fprintf(stderr, "index %ld(%%r%ld)\n", (long)token->data.ndx.val,
                    (long)token->data.ndx.reg);

            break;
    }
}

void
vasfreetoken(struct vastoken *token)
{
#if (VASDB)
    free(token->file);
#endif
    free(token);

    return;
}

static void
vasqueuetoken(struct vastoken *token)
{
    token->next = NULL;
    if (!vastokenqueue) {
        token->prev = NULL;
        vastokenqueue = token;
    } else if (vastokentail) {
        token->prev = vastokentail;
        vastokentail->next = token;
        vastokentail = token;
    } else {
        vastokenqueue->next = token;
        token->prev = vastokenqueue;
        vastokentail = token;
    }

    return;
}

#if (VASDB)

void
vasaddline(vasmemadr_t adr, uint8_t *data, uint8_t *filename, unsigned long line)
{
    struct vasline *newline = malloc(sizeof(struct vasline));
    unsigned long   key;

    key = (adr & 0xff) + ((adr >> 8) & 0xff) + ((adr >> 16) & 0xff) + ((adr >> 24) & 0xff);
    newline->adr = adr;
    newline->file = strdup(filename);
    newline->num = line;
    newline->data = data;
    newline->next = vaslinehash[key];
    key &= (VASNHASH - 1);
    vaslinehash[key] = newline;

    return;
}

struct vasline *
vasfindline(vasmemadr_t adr)
{
    struct vasline *line;
    unsigned long   key;

    key = (adr & 0xff) + ((adr >> 8) & 0xff) + ((adr >> 16) & 0xff) + ((adr >> 24) & 0xff);
    key &= (VASNHASH - 1);
    line = vaslinehash[key];
    while ((line) && line->adr != adr) {
        line = line->next;
    }

    return line;
}

#endif

void
vasadddef(struct vasdef *def)
{
    unsigned long   key = 0;
    char           *ptr;

    ptr = def->name;
    while (*ptr) {
        key += *ptr++;
    }
    key &= (VASNHASH - 1);
    def->next = vasdefhash[key];
    vasdefhash[key] = def;

    return;
}

struct vasdef *
vasfinddef(char *str, vasword_t *defptr, char **retptr)
{
    unsigned long  key = 0;
    struct vasdef *def = NULL;
    char          *ptr;
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
        key &= (VASNHASH - 1);
        def = vasdefhash[key];
        while ((def) && strncmp(def->name, ptr, len)) {
            def = def->next;
        }
        if (def) {
            *defptr = def->val;
            *retptr = str;
        }
    }

    return def;
}

void
vasaddval(struct vasval *val)
{
    unsigned long   key = 0;
    char           *ptr;

    ptr = val->name;
    while (*ptr) {
        key += *ptr++;
    }
    key &= (VASNHASH - 1);
    val->next = vasvalhash[key];
    vasvalhash[key] = val;

    return;
}

struct vasval *
vasfindval(char *str, vasword_t *valptr, char **retptr)
{
    unsigned long  key = 0;
    struct vasval *val = NULL;
    char          *ptr;
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
        key &= (VASNHASH - 1);
        val = vasvalhash[key];
        while ((val) && strncmp(val->name, ptr, len)) {
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
vasqueuesym(struct vassymrec *sym)
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
vasaddsym(struct vassymrec *sym)
{
    char          *str = sym->name;
    unsigned long  key = 0;

    while (*str) {
        key += *str++;
    }
#if (VASZEROHASH)
#if (WORDSIZE == 8)
    key = tmhash64(key);
#else
    key = tmhash32(key);
#endif
#endif
    key &= (VASNHASH - 1);
    sym->next = vassymhash[key];
    vassymhash[key] = sym;

    return;
}

struct vassymrec *
vasfindsym(char *name)
{
    struct vassymrec *sym = NULL;
    char             *str = name;
    unsigned long     key = 0;

    while (*str) {
        key += *str++;
    }
#if (VASZEROHASH)
#if (WORDSIZE == 8)
    key = tmhash64(key);
#else
    key = tmhash32(key);
#endif
#endif
    key &= (VASNHASH - 1);
    sym = vassymhash[key];
    while ((sym) && strcmp(sym->name, name)) {
        sym = sym->next;
    }

    return sym;
}

void
vasremovesyms(void)
{
    struct vassymrec *sym1;
    struct vassymrec *sym2;
    long              l;

    for (l = 0 ; l < VASNHASH ; l++) {
        sym1 = vassymhash[l];
        while (sym1) {
            sym2 = sym1;
            sym1 = sym1->next;
            free(sym2);
        }
        vassymhash[l] = NULL;
    }

    return;
}

static void
vasaddglob(struct vaslabel *label)
{
    char          *str = label->name;
    unsigned long  key = 0;

    while (*str) {
        key += *str++;
    }
    key &= (VASNHASH - 1);
    label->next = vasglobhash[key];
    vasglobhash[key] = label;

    return;
}

struct vaslabel *
vasfindglob(char *name)
{
    struct vaslabel *label = NULL;
    char            *str = name;
    unsigned long    key = 0;

    while (*str) {
        key += *str++;
    }
    key &= (VASNHASH - 1);
    label = vasglobhash[key];
    while ((label) && strcmp(label->name, name)) {
        label = label->next;
    }

    return label;
}

static char *
vasgetlabel(char *str, char **retptr)
{
    char *ptr = str;
    char *name = NULL;

#if (VASDEBUG)
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
        name = strdup(ptr);
        *retptr = str;
    }

    return name;
}

static struct vasop *
vasgetinst(char *str, char **retptr)
{
    struct vasop *op;

    op = vasfindop(str);
#if (VASDEBUG)
    fprintf(stderr, "getinst: %s\n", str);
#endif
    if (op) {
        str += op->len;
        *retptr = str;
    }

    return op;
}

static char *
vasgetdef(char *str, char **retptr)
{
    char *ptr = str;
    char *name = NULL;

#if (VASDEBUG)
    fprintf(stderr, "getdef: %s\n", str);
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
    if (*str == '=') {
        *str++ = '\0';
    }
    name = strdup(ptr);
    *retptr = str;

    return name;
}

static char *
vasgetsym(char *str, char **retptr)
{
    char *ptr = str;
    char *name = NULL;

#if (VASDEBUG)
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
    name = strdup(ptr);
    *retptr = str;

    return name;
}

static long
vasgetvalue(char *str, vasword_t *valret, char **retstr)
{
    long                found = 0;
    vasuword_t          uval = 0;
    vasword_t           val = 0;
    long                neg = 0;

#if (VASDEBUG)
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
            val = -((vasword_t)uval);
            *valret = val;
        } else {
            *valret = (vasword_t)uval;
        }
    }

    return found;
}

static char *
vasgetadr(char *str, char **retptr)
{
    char *ptr = str;
    char *name = NULL;

#if (VASDEBUG)
    fprintf(stderr, "getadr: %s\n", str);
#endif
    while (isalpha(*str) || *str == '_') {
        str++;
    }
    if (*str == ',') {
        *str++ = '\0';
    }
    name = strdup(ptr);
    *retptr = str;

    return name;
}

static vasuword_t
vasgetindex(char *str, vasword_t *retndx, char **retptr)
{
    vasuword_t reg = 0xff;
    vasword_t  val = 0;
    vasword_t  ndx = 0;
    vasword_t  size = 0;
    long       neg = 0;

#if (VASDEBUG)
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
        reg = vasgetreg(str, &size, &str);
#if 0
        if (reg >= VASNREG) {
            fprintf(stderr, "invalid register name %s\n", str);

            exit(1);
        }
#endif
        *retptr = str;
    }
#if (VASNEWHACKS)
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

static int
vasgetchar(char *str, char **retptr)
{
    char      *name = str;
    vasword_t  val = 0;

#if (VASDEBUG)
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

    return (unsigned char)val;
}

#if (VASMMAP)
static struct vastoken *
vasgettoken(char *str, char **retptr, struct vasmap *map)
#else
static struct vastoken *
vasgettoken(char *str, char **retptr)
#endif
{
    long             buflen = VASLINELEN;
    long             len;
    char            *buf = vasstrbuf;
    struct vastoken *token1 = malloc(sizeof(struct vastoken));
    struct vastoken *token2;
    struct vasop    *op = NULL;
    char            *name = str;
    vasword_t        val = VASRESOLVE;
    vasword_t        size = 0;
    vasword_t        ndx;
    int              ch;
#if (VASDB)
    char            *ptr;
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
#if (VASDEBUG)
    fprintf(stderr, "gettoken: %s\n", str);
#endif
    if ((*str) && (isdigit(*str) || *str == '-')) {
        val = vasgetindex(str, &ndx, &str);
        if (val < 0xff) {
            token1->type = VASTOKENINDEX;
            token1->data.ndx.reg = VASREGINDEX | val;
            token1->data.ndx.val = ndx;
        } else if (vasgetvalue(str, &val, &str)) {
            token1->type = VASTOKENVALUE;
            token1->data.value.val = val;
            token1->data.value.size = size;
        } else {
            fprintf(stderr, "invalid token %s\n", vaslinebuf);

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
                        len, buflen, vaslinebuf);

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
            token1->type = VASTOKENSTRING;
            token1->data.str = strdup(vasstrbuf);
        }
    } else if ((*str) && *str == '%') {
        str++;
        val = vasgetreg(str, &size, &str);
        token1->type = VASTOKENREG;
#if (VASNEWHACKS)
        token1->size = size;
#endif
        token1->data.reg = val;
    } else if ((*str) && (isalpha(*str) || *str == '_')) {
        name = vasgetlabel(str, &str);
        if (name) {
            token1->type = VASTOKENLABEL;
            token1->data.label.name = name;
            token1->data.label.adr = VASRESOLVE;
        } else {
#if (VASDB)
            ptr = str;
#endif
            op = vasgetinst(str, &str);
            if (op) {
                token1->type = VASTOKENINST;
                token1->data.inst.name = op->name;
                token1->data.inst.op = op->code;
                token1->data.inst.narg = op->narg;
#if (VASDB)
                token1->data.inst.data = strdup(ptr);
#endif
            } else {
                name = vasgetsym(str, &str);
                if (name) {
                    token1->type = VASTOKENSYM;
                    token1->data.sym.name = name;
                    token1->data.sym.adr = VASRESOLVE;
                }
                if (!name) {
                    name = vasgetadr(str, &str);
                    if (name) {
                        token1->type = VASTOKENSYM;
                        token1->data.sym.name = name;
                        token1->data.sym.adr = VASRESOLVE;
                    } else {
                        fprintf(stderr, "invalid token %s\n", vaslinebuf);

                        exit(1);
                    }
                }
            }
        }
    } else if ((*str) && *str == '$') {
        str++;
        if (isalpha(*str) || *str == '_') {
            if (vasfinddef(str, &val, &str)) {
                token1->type = VASTOKENDEF;
                token1->val = val;
            } else if ((*str) && (isalpha(*str)
                                 || *str == '_' || *str == '-')) {
                if (vasfindval(str, &val, &str)) {
                    token1->type = VASTOKENIMMED;
                    token1->val = val;
                } else if (vasgetvalue(str, &val, &str)) {
                    token1->type = VASTOKENIMMED;
                    token1->val = val;
                } else {
                    name = vasgetsym(str, &str);
                    if (name) {
                        token1->type = VASTOKENADR;
                        token1->data.adr.name = name;
                        token1->data.adr.val = VASRESOLVE;
                    } else {
                        fprintf(stderr, "invalid token %s\n", vaslinebuf);

                        exit(1);
                    }
                }
            }
        } else if ((*str) && isdigit(*str)) {
            if (vasgetvalue(str, &val, &str)) {
                token1->type = VASTOKENIMMED;
                token1->val = val;
                while (isspace(*str)) {
                    str++;
                }
                if (*str == ',') {
                    str++;
                }
            } else {
                fprintf(stderr, "invalid immediate %s\n", str);

                exit(1);
            }
        }
    } else if (*str == '\'') {
        val = vasgetchar(str, &str);
        token1->type = VASTOKENCHAR;
        token1->data.ch = val;
    } else if (*str == '.') {
        str++;
        size = 0;
        if (!strncmp(str, "quad", 4)) {
            str += 4;
            token1->type = VASTOKENDATA;
            size = token1->data.size = 8;
        } else if (!strncmp(str, "long", 4)) {
            str += 4;
            token1->type = VASTOKENDATA;
            size = token1->data.size = 4;
        } else if (!strncmp(str, "byte", 4)) {
            str += 4;
            token1->type = VASTOKENDATA;
            size = token1->data.size = 1;
        } else if (!strncmp(str, "short", 5)) {
            str += 5;
            token1->type = VASTOKENDATA;
            size = token1->data.size = 2;
        } else if (!strncmp(str, "globl", 5)) {
            str += 5;
            token1->type = VASTOKENGLOBL;
        } else if (!strncmp(str, "space", 5)) {
            str += 5;
            token1->type = VASTOKENSPACE;
        } else if (!strncmp(str, "org", 3)) {
            str += 3;
            token1->type = VASTOKENORG;
        } else if (!strncmp(str, "align", 5)) {
            str += 5;
            token1->type = VASTOKENALIGN;
        } else if (!strncmp(str, "asciz", 5)) {
            str += 5;
            token1->type = VASTOKENASCIZ;
        }
    } else if (*str == '*' || *str == '(') {
        str++;
        token1->type = VASTOKENINDIR;
        token2 = malloc(sizeof(struct vastoken));
        if (*str == '%') {
            str++;
            val = vasgetreg(str, &size, &str);
            token2->type = VASTOKENREG;
#if (VASNEWHACKS)
            token2->size = size;
#endif
            token2->data.reg = VASREGINDIR | val;
            vasqueuetoken(token1);
            token1 = token2;
        } else if (isalpha(*str) || *str == '_') {
            name = vasgetsym(str, &str);
            if (name) {
                token2->type = VASTOKENSYM;
                token2->data.sym.name = name;
                vasqueuetoken(token1);
                token1 = token2;
            } else {
                fprintf(stderr, "invalid token %s\n", vaslinebuf);

                exit(1);
            }
        } else {
            fprintf(stderr, "invalid token %s\n", vaslinebuf);

            exit(1);
        }
    } else if (isalpha(*str) || *str == '_') {
        name = vasgetdef(str, &str);
        if (name) {
            while (isspace(*str)) {
                str++;
            }
            if (vasgetvalue(str, &val, &str)) {
                token1->type = VASTOKENDEF;
                token1->data.def.name = name;
                token1->data.def.val = val;
            }
        }
    }
    *retptr = str;

    return token1;
}

static struct vastoken *
vasprocvalue(struct vastoken *token, vasmemadr_t adr,
             vasmemadr_t *retadr)
{
    vasmemadr_t      ret = adr + token->data.value.size;
    char            *valptr = vasadrtoptr(adr);
    struct vastoken *retval;

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
    vasfreetoken(token);

    return retval;
}

static struct vastoken *
vasproclabel(struct vastoken *token, vasmemadr_t adr,
             vasmemadr_t *retadr)
{
    struct vassymrec *sym;
    struct vastoken  *retval;

    if (!_startset && !strncmp(token->data.label.name, "_start", 6)) {
#if (WPMTRACE)
        fprintf(stderr, "_start == 0x%08x\n", adr);
#endif
        if (adr & (sizeof(vasop_t) - 1)) {
            adr = rounduppow2(adr, sizeof(vasop_t));
        }
        _startadr = adr;
        _startset = 1;
    }
    sym = vasfindsym(token->data.label.name);
    if (sym) {
        sym->adr = adr;
    } else {
        sym = malloc(sizeof(struct vassymrec));
        sym->name = token->data.label.name;
        sym->adr = adr;
        vasaddsym(sym);
    }
    *retadr = adr;
    retval = token->next;
    vasfreetoken(token);

    return retval;
}

static struct vastoken *
vasprocchar(struct vastoken *token, vasmemadr_t adr,
            vasmemadr_t *retadr)
{
    char            *valptr = vasadrtoptr(adr);
    struct vastoken *retval;

    *valptr = token->data.ch;
    adr++;
    *retadr = adr;
    retval = token->next;
    vasfreetoken(token);

    return retval;
}

static struct vastoken *
vasprocdata(struct vastoken *token, vasmemadr_t adr,
            vasmemadr_t *retadr)
{
    struct vastoken *token1 = token->next;
    vasmemadr_t      valadr = adr;

    while ((token1) && token1->type == VASTOKENVALUE) {
        token1->data.value.size = token->data.size;
        token1 = vasprocvalue(token1, valadr, &valadr);
    }
    *retadr = valadr;

    return token1;
}

static struct vastoken *
vasprocglobl(struct vastoken *token, vasmemadr_t adr,
             vasmemadr_t *retadr)
{
    struct vastoken *token1;
    struct vaslabel *label;

    token1 = token->next;
    while ((token1) && token1->type == VASTOKENSYM) {
        token1 = vasproclabel(token1, adr, &adr);
        label = malloc(sizeof(label));
        label->name = strdup(token1->data.label.name);
        label->adr = adr;
        vasaddglob(label);
    }
    *retadr = adr;

    return token1;
}

static struct vastoken *
vasprocspace(struct vastoken *token, vasmemadr_t adr,
             vasmemadr_t *retadr)
{
    struct vastoken *token1;
    struct vastoken *token2;
    vasmemadr_t      spcadr;
    char            *ptr;
    uint8_t          val;

    token1 = token->next;
    if ((token1) && token1->type == VASTOKENVALUE) {
        spcadr = token1->data.value.val;
        token2 = token1->next;
        if ((token2) && token2->type == VASTOKENVALUE) {
            ptr = vasadrtoptr(spcadr);
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

static struct vastoken *
vasprocorg(struct vastoken *token, vasmemadr_t adr,
           vasmemadr_t *retadr)
{
    struct vastoken *token1;
    vasmemadr_t      orgadr;
    char            *ptr;
    uint8_t          val;

    token1 = token->next;
    if ((token1) && token1->type == VASTOKENVALUE) {
        ptr = vasadrtoptr(adr);
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

static struct vastoken *
vasprocalign(struct vastoken *token, vasmemadr_t adr,
             vasmemadr_t *retadr)
{
    struct vastoken *token1;

    token1 = token->next;
    if ((token1) && token1->type == VASTOKENVALUE) {
        adr = rounduppow2(adr, token1->data.value.val);
    } else {
        fprintf(stderr, "invalid .align attribute token type %lx\n",
                token1->type);

        exit(1);
     }
    *retadr = adr;

    return token1;
}

static struct vastoken *
vasprocasciz(struct vastoken *token, vasmemadr_t adr,
             vasmemadr_t *retadr)
{
    struct vastoken *token1;
    struct vastoken *token2;
    long             len = 0;
    char            *ptr;
    char            *str;

    token1 = token->next;
    while ((token1) && token1->type == VASTOKENSTRING) {
        ptr = vasadrtoptr(adr);
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
        vasfreetoken(token2);
     }
    *retadr = adr;

    return token1;
}

void
vasinit(void)
{
    vasinitbuf();
#if (VASALIGN)
    vasinitalign();
#endif
}

vasmemadr_t
vastranslate(vasmemadr_t base)
{
    vasmemadr_t      adr = base;
    struct vastoken *token = vastokenqueue;
    struct vastoken *token1;
    vastokfunc_t    *func;

    while (token) {
        func = vasktokfunctab[token->type];
        if (func) {
#if (VASALIGN)
            adr = vasaligntok(adr, token->type);
#endif
            token1 = func(token, adr, &adr);
            if (!token) {

                break;
            }
        } else {
            fprintf(stderr, "stray token of type %lx\n", token->type);
            vasprinttoken(token);

            exit(1);
        }
        token = token1;
    }

    return adr;
}

void
vasresolve(vasmemadr_t base)
{
    struct vassymrec *sym = symqueue;
    struct vassymrec *sym1;
    struct vassymrec *item;
    struct vaslabel  *label;

    while (sym) {
        if (sym->adr == VASRESOLVE) {
            fprintf(stderr, "unresolved symbol %s\n", sym->name);

            exit(1);
        }
        item = vasfindsym(sym->name);
        if (item) {
            if (item->adr == VASRESOLVE) {
                fprintf(stderr, "invalid symbol %s\n", item->name);

                exit(1);
            }
            *((vasmemadr_t *)sym->adr) = item->adr;
        } else {
            label = vasfindglob(sym->name);
            if (label) {
                *((vasmemadr_t *)sym->adr) = label->adr;
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

#if (VASBUF)
void
vasreadfile(char *name, vasmemadr_t adr, int bufid)
#else
void
vasreadfile(char *name, vasmemadr_t adr)
#endif
{
#if (VASMMAP)
    struct vasmap    map;
    struct stat      statbuf;
    int              sysret;
    int              fd;
    uint8_t         *base;
#endif
    long             buflen = VASLINELEN;
#if (VASBUF)
    int              fd = -1;
#else
    FILE            *fp = fopen(name, "r");
#endif
    long             eof = 0;
    struct vastoken *token = NULL;
    struct vasval   *def;
    char            *fname;
    char            *ptr;
    char            *str = vaslinebuf;
    char            *lim = NULL;
    long             loop = 1;
    int              ch;
    long             comm = 0;
    long             done = 1;
    long             len = 0;
#if (VASDB)
    unsigned long    line = 0;
#endif
    vasword_t        val = 0;

#if (VASBUF) || (VASMMAP)
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
#if (VASMMAP)
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
#if (VASMMAP)
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
            str = vaslinebuf;
            done = 0;
#if (VASMMAP)
            ch = vasgetc(&map);
#elif (VASBUF)
            ch = vasgetc(fd, bufid);
#else
            ch = fgetc(fp);
#endif
            if (ch == EOF) {
                loop = 0;

                break;
            } else {
                len = 0;
#if (VASDB)
                line++;
#endif
                while (ch != EOF && ch != '\n') {
                    *str++ = (uint8_t)ch;
                    len++;
                    if (len == buflen) {
                        fprintf(stderr, "overlong line (%ld == %ld): %s\n",
                                len, buflen, vaslinebuf);

                        exit(1);
                    }
#if (VASMMAP)
                    ch = vasgetc(&map);
#elif (VASBUF)
                    ch = vasgetc(fd, bufid);
#else
                    ch = fgetc(fp);
#endif
                }
                eof = (ch == EOF);
                *str = '\0';
                str = vaslinebuf;
                lim = str + len;
                while ((*str) && isspace(*str)) {
                    str++;
                }
                if (str > lim) {
                    done = 1;
                }
            }
        } else if (!strncmp(str, ".define", 7)) {
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
                if (vasgetvalue(str, &val, &str)) {
                    def = malloc(sizeof(struct vasval));
                    def->name = strdup(ptr);
                    def->val = val;
                    vasaddval(def);
                } else {
                    fprintf(stderr, "invalid .define directive %s\n", ptr);

                    exit(1);
                }
            }
        } else if (!strncmp(str, ".include", 8)) {
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
#if (VASBUF)
                    vasreadfile(fname, adr, ++bufid);
                    bufid--;
#else
                    vasreadfile(fname, adr);
#endif
                    vasresolve(adr);
                    vasremovesyms();
                } else {
                    fprintf(stderr, "invalid .include directive %s\n",
                            str);

                    exit(1);
                }
            }
            done = 1;
        } else if (!strncmp(str, ".import", 7)) {
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
#if (VASBUF)
                    vasreadfile(fname, adr, ++bufid);
                    bufid--;
#else
                    vasreadfile(fname, adr);
#endif
                    vasresolve(adr);
                    vasremovesyms();
                } else {
                    fprintf(stderr, "invalid .import directive %s\n",
                            vasstrbuf);

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
#if (VASMMAP)
                ch = vasgetc(&map);
#elif (VASBUF)
                ch = vasgetc(fd, bufid);
#else
                ch = fgetc(fp);
#endif
                if (ch == EOF) {
                    loop = 0;

                    break;
#if (VASDB)
                } else if (ch == '\n') {
                    line++;
#endif
                } else if (ch == '*') {
#if (VASMMAP)
                    ch = vasgetc(&map);
#elif (VASBUF)
                    ch = vasgetc(fd, bufid);
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
#if (VASMMAP)
                token = vasgettoken(str, &str, &map);
#else
                token = vasgettoken(str, &str);
#endif
                if (token) {
#if (VASDB)
                    token->file = strdup(name);
                    token->line = line;
#endif
                    vasqueuetoken(token);
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
#if (VASBUF)
    close(fd);
#else
    fclose(fp);
#endif
#if (VASMMAP)
    munmap(map.adr, map.sz);
#endif

    return;
}

