#define ZASDEBUG   0
#define ZASBUFSIZE 131072

#if (WPM)
#include <wpm/conf.h>
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
#if (ZASMMAP)
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#elif (ZASBUF)
#include <errno.h>
#include <fcntl.h>
#endif
#if (ZASPROF)
#include <zero/prof.h>
#endif
#include <zas/zas.h>
#if (ZPC)
#include <zpc/zpc.h>
#include <zpc/asm.h>
#endif
#include <wpm/mem.h>
#include <wpm/wpm.h>

#if (ZASMMAP)
struct zasmap {
    uint8_t *adr;
    uint8_t *cur;
    uint8_t *lim;
    size_t   sz;
};
#endif

typedef struct zastoken * zastokfunc_t(struct zastoken *, zasmemadr_t, zasmemadr_t *);

static zasuword_t        zasgetreg(uint8_t *str, uint8_t **retptr);
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
static struct zastoken * zasprocinst(struct zastoken *, zasmemadr_t, zasmemadr_t *);
static struct zastoken * zasprocchar(struct zastoken *, zasmemadr_t, zasmemadr_t *);
static struct zastoken * zasprocdata(struct zastoken *, zasmemadr_t, zasmemadr_t *);
static struct zastoken * zasprocglobl(struct zastoken *, zasmemadr_t, zasmemadr_t *);
static struct zastoken * zasprocspace(struct zastoken *, zasmemadr_t, zasmemadr_t *);
static struct zastoken * zasprocorg(struct zastoken *, zasmemadr_t, zasmemadr_t *);
static struct zastoken * zasprocalign(struct zastoken *, zasmemadr_t, zasmemadr_t *);
static struct zastoken * zasprocasciz(struct zastoken *, zasmemadr_t, zasmemadr_t *);

/* TODO: combine these into a structure table for better cache locality */
//static char    **zasopnametab;
//static uint8_t  *zasopnargtab;
static struct zasopinfo *zasopinfotab;
#if (ZPC)
extern char     *zpcopnametab[ZPCNASMOP];
extern uint8_t   zpcopnargtab[ZPCNASMOP];
#else
extern struct zasopinfo wpmopinfotab[WPMNASMOP + 1];
#endif

struct zassymrec {
    struct zassymrec *next;
    uint8_t          *name;
    uintptr_t         adr;
};

#define NHASHITEM 1024
static struct zasop     *ophash[NHASHITEM] ALIGNED(PAGESIZE);
static struct zassymrec *symhash[NHASHITEM];
static struct zasval    *valhash[NHASHITEM];
static struct zaslabel  *globhash[NHASHITEM];
#if (ZASDB) || (WPMDB)
struct zasline          *linehash[NHASHITEM];
#endif

zastokfunc_t *zasktokfunctab[NTOKTYPE]
= {
    NULL,
    zasprocvalue,
    zasproclabel,
    zasprocinst,
    NULL,               // REG
    NULL,               // SYM
    zasprocchar,
    NULL,               // IMMED
    NULL,               // INDIR
    NULL,               // ADR
    NULL,               // INDEX
    zasprocdata,
    zasprocglobl,
    zasprocspace,
    zasprocorg,
    zasprocalign,
    zasprocasciz
};

struct zastoken         *zastokenqueue;
struct zastoken         *zastokentail;
static struct zassymrec *symqueue;
zasmemadr_t              _startadr;
static zasmemadr_t       _startset;
unsigned long            zasinputread;
static uint8_t          *linebuf;
static uint8_t          *strbuf;
#if (ZASBUF) && (!ZASMMAP)
struct readbuf {
    void    *data;
    uint8_t *cur;
    uint8_t *lim;
};
struct readbuf          *readbuftab;
static long              nreadbuf = 16;
long                     readbufcur = 0;
#endif

#if (ZASMMAP)
#define zasgetc(map) ((map)->cur < (map)->lim ? *(map)->cur++ : EOF)
#if 0
static int
zasgetc(struct zasmap *map)
{
    int ch = EOF;

    if (map->cur < map->adr + map->sz) {
        ch = *map->cur++;
    }

    return ch;
}
#endif
#elif (ZASBUF)
static int
zasgetc(int fd, int bufid)
{
    struct readbuf *buf = &readbuftab[bufid];
    ssize_t         nleft = ZASBUFSIZE;
    ssize_t         n;
    int             ch = EOF;
    long            l = nreadbuf;

    if (bufid >= nreadbuf) {
        nreadbuf <<= 1;
        readbuftab = realloc(readbuftab, nreadbuf * sizeof(struct readbuf));
        for ( ; l < nreadbuf ; l++) {
            readbuftab[l].data = malloc(ZASBUFSIZE);
        }
    }
    if (buf->cur < buf->lim) {
        ch = *buf->cur++;
    } else if (buf->cur == buf->lim) {
        n = 0;
        while (nleft) {
            n = read(fd, buf->data, ZASBUFSIZE);
            if (n < 0) {
                if (errno == EINTR) {
                    
                    continue;
                } else {
                    
                    return EOF;
                }
            } else if (n == 0) {

                break;
            } else {
                nleft -= n;
            }
        }
        if (nleft == ZASBUFSIZE) {

            return EOF;
        }
        buf->cur = buf->data;
        buf->lim = (uint8_t *)buf->data + ZASBUFSIZE - nleft;
        ch = *buf->cur++;
    }

    return ch;
}
#endif

void
printtoken(struct zastoken *token)
{
    switch (token->type) {
        case TOKENVALUE:
#if (ZPC)
            zpcprinttoken(token->token);
#else
            fprintf(stderr, "value 0x%08x (size == %d)\n",
                    token->data.value.val, token->data.value.size);
#endif

            break;
        case TOKENLABEL:
            fprintf(stderr, "label %s (adr == 0x%08lx)\n",
                    token->data.label.name,
                    (unsigned long)token->data.label.adr);

            break;
        case TOKENINST:
            fprintf(stderr, "instruction %s (op == 0x%02x)\n",
                    token->data.inst.name, token->data.inst.op);

            break;
        case TOKENREG:
            fprintf(stderr, "register r%1lx\n", (long)token->data.ndx.reg);

            break;
        case TOKENSYM:
            fprintf(stderr, "symbol %s (adr == 0x%08lx)\n",
                    token->data.sym.name, (long)token->data.sym.adr);

            break;
        case TOKENCHAR:
            fprintf(stderr, "character 0x%02x\n", token->data.ch);

            break;
        case TOKENIMMED:
            fprintf(stderr, "immediate (val == 0x%08lx)\n", (long)token->val);

            break;
        case TOKENINDIR:
            fprintf(stderr, "indirection\n");

            break;
        case TOKENADR:
            fprintf(stderr, "address (sym == %s, adr == 0x%08lx)\n",
                    token->data.adr.name, (long)token->data.adr.val);

            break;
        case TOKENINDEX:
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
#if (ZPC)
    free(token->token);
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

static void
zasaddop(struct zasop *op)
{
    uint8_t       *str = op->name;
    unsigned long  key = 0;
    unsigned long  len = 0;

    while (*str) {
        key += *str++;
        len++;
    }
    op->len = len;
    key &= (NHASHITEM - 1);
    op->next = ophash[key];
    ophash[key] = op;

    return;
}

struct zasop *
zasfindop(uint8_t *name)
{
    struct zasop  *op = NULL;
    uint8_t       *str = name;
    unsigned long  key = 0;

    while ((*str) && isalpha(*str)) {
        key += *str++;
    }
    key &= (NHASHITEM - 1);
    op = ophash[key];
    while ((op) && strncmp((char *)op->name, (char *)name, op->len)) {
        op = op->next;
    }

    return op;
}

#if (ZASDB) || (WPMDB)
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
    newline->next = linehash[key];
    key &= (NHASHITEM - 1);
    linehash[key] = newline;

    return;
}

struct zasline *
zasfindline(zasmemadr_t adr)
{
    struct zasline *line;
    unsigned long   key;

    key = (adr & 0xff) + ((adr >> 8) & 0xff) + ((adr >> 16) & 0xff) + ((adr >> 24) & 0xff);
    key &= (NHASHITEM - 1);
    line = linehash[key];
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
    key &= (NHASHITEM - 1);
    val->next = valhash[key];
    valhash[key] = val;

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
        key &= (NHASHITEM - 1);
        val = valhash[key];
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

static void
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
    key &= (NHASHITEM - 1);
    sym->next = symhash[key];
    symhash[key] = sym;

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
    key &= (NHASHITEM - 1);
    sym = symhash[key];
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

    for (l = 0 ; l < NHASHITEM ; l++) {
        sym1 = symhash[l];
        while (sym1) {
            sym2 = sym1;
            sym1 = sym1->next;
            free(sym2);
        }
        symhash[l] = NULL;
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
    key &= (NHASHITEM - 1);
    label->next = globhash[key];
    globhash[key] = label;

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
    key &= (NHASHITEM - 1);
    label = globhash[key];
    while ((label) && strcmp((char *)label->name, (char *)name)) {
        label = label->next;
    }

    return label;
}

void
zasinitop(void)
{
    struct zasop *op;
    long       l;

    for (l = 1 ; (zasopinfotab[l].name) ; l++) {
        op = malloc(sizeof(struct zasop));
        op->name = (uint8_t *)zasopinfotab[l].name;
        op->code = (uint8_t)l;
        op->narg = zasopinfotab[l].narg;
        zasaddop(op);
    }

    return;
}

void
zasinitbuf(void)
{
#if (ZASBUF)
    long l;
#endif

    linebuf = malloc(LINELEN);
    strbuf = malloc(LINELEN);
#if (ZASBUF)
    readbuftab = malloc(nreadbuf * sizeof(struct readbuf));
    for (l = 0 ; l < nreadbuf ; l++) {
        readbuftab[l].data = malloc(ZASBUFSIZE);
    }
#endif

    return;
}

static zasuword_t
zasgetreg(uint8_t *str, uint8_t **retptr)
{
    zasuword_t reg = 0;

#if (ZASDEBUG)
    fprintf(stderr, "getreg: %s\n", str);
#endif
    if (*str == 'r') {
        str++;
        while ((*str) && isdigit(*str)) {
            reg *= 10;
            reg += *str - '0';
            str++;
        }
        while (*str == ')' || *str == ',') {
            str++;
        }
        *retptr = str;
    } else {
        fprintf(stderr, "invalid register name %s\n", str);
        
        exit(1);
    }

    return reg;
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

    op = zasfindop(str);
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
zasgetvalue(uint8_t *str, zasword_t *retval, uint8_t **retptr)
{
    long      found = 0;
    zasword_t val = 0;
    long      neg = 0;

#if (ZASDEBUG)
    fprintf(stderr, "getvalue: %s\n", str);
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
        found = 1;
    } else if (str[0] == '0' && (str[1] == 'b' || str[1] == 'B')) {
        str += 2;
        while ((*str) && (*str == '0' || *str == '1')) {
            val <<= 1;
            val += *str - '0';
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
            val <<= 3;
            val += *str - '0';
            str++;
        }
        found = 1;
    } else if (isdigit(*str)) {
        while ((*str) && isdigit(*str)) {
            val *= 10;
            val += *str - '0';
            str++;
        }
        found = 1;
    }
    if (found) {
        if (neg) {
            val = -val;
        }
        *retval = val;
        while (*str == ',' || *str == '\'') {
            str++;
        }
        *retptr = str;
    }

    return found;
}

static zasuword_t
zasgetindex(uint8_t *str, zasword_t *retndx, uint8_t **retptr)
{
    zasuword_t reg = 0xff;
    zasword_t  val = 0;
    zasword_t  ndx = 0;
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
#if (ZPC)
        reg = zpcgetreg(str, &str);
        if ((reg & 0x0f) >= ZPCNREG) {
            fprintf(stderr, "invalid register name %s\n", str);

            exit(1);
        }
#else
        reg = zasgetreg(str, &str);
        if (reg >= NREG) {
            fprintf(stderr, "invalid register name %s\n", str);

            exit(1);
        }
#endif
        *retptr = str;
    }
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
    long             buflen = LINELEN;
    long             len;
    uint8_t         *buf = strbuf;
    struct zastoken *token1 = malloc(sizeof(struct zastoken));
    struct zastoken *token2;
#if (ZPC)
    struct zpctoken *zpctoken;
#endif
    struct zasop    *op = NULL;
    uint8_t         *name = str;
    zasword_t        val = RESOLVE;
    long             size = 0;
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
#if (ZPC)
        if (val < ZPCNREG) {
            token1->type = TOKENINDEX;
            token1->data.ndx.reg = ZPCREGINDEX | val;
            token1->data.ndx.val = ndx;
        } else {
            zpctoken = zpcgettoken((const char *)str, (char **)&str);
            if (zpctoken) {
                token1->type = TOKENVALUE;
                token1->token = zpctoken;
#if 0
                token1->data.value.val = val;
                token1->data.value.size = size;
#endif
            } else {
                fprintf(stderr, "invalid token %s\n", linebuf);
                
                exit(1);
            }
        }
#else
        if (val < NREG) {
            token1->type = TOKENINDEX;
            token1->data.ndx.reg = REGINDEX | val;
            token1->data.ndx.val = ndx;
        } else if (zasgetvalue(str, &val, &str)) {
            token1->type = TOKENVALUE;
            token1->data.value.val = val;
            token1->data.value.size = size;
        } else {
            fprintf(stderr, "invalid token %s\n", linebuf);
                
            exit(1);
        }
#endif
    } else if ((*str) && *str == '"') {
        str++;
        len = 0;
        while (*str) {
            *buf++ = *str++;
            len++;
            if (len == buflen) {
                fprintf(stderr, "overlong line (%ld == %ld): %s\n",
                        len, buflen, linebuf);

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
            token1->type = TOKENSTRING;
            token1->data.str = (uint8_t *)strdup((char *)strbuf);
        }
    } else if ((*str) && *str == '%') {
        str++;
#if (ZPC)
        val = zpcgetreg(str, &str);
        if (val & ZPCREGSTKBIT) {
            token1->token->param = ZPCCALC;
            val &= ~ZPCREGSTKBIT;
        }
#else
        val = zasgetreg(str, &str);
#endif
        token1->type = TOKENREG;
        token1->data.reg = val;
    } else if ((*str) && (isalpha(*str) || *str == '_')) {
        name = zasgetlabel(str, &str);
        if (name) {
            token1->type = TOKENLABEL;
            token1->data.label.name = name;
            token1->data.label.adr = RESOLVE;
        } else {
#if (ZASDB)
            ptr = str;
#endif
            op = zasgetinst(str, &str);
            if (op) {
                token1->type = TOKENINST;
                token1->data.inst.name = op->name;
                token1->data.inst.op = op->code;
                token1->data.inst.narg = op->narg;
#if (ZASDB)
                token1->data.inst.data = (uint8_t *)strdup((char *)ptr);
#endif
            } else {
                name = zasgetsym(str, &str);
                if (name) {
                    token1->type = TOKENSYM;
                    token1->data.sym.name = name;
                    token1->data.sym.adr = RESOLVE;
                }
                if (!name) {
                    name = zasgetadr(str, &str);
                    if (name) {
                        token1->type = TOKENSYM;
                        token1->data.sym.name = name;
                        token1->data.sym.adr = RESOLVE;
                    } else {
                        fprintf(stderr, "invalid token %s\n", linebuf);
                        
                        exit(1);
                    }
                }
            }
        }
    } else if ((*str) && *str == '$') {
        str++;
        if ((*str) && (isalpha(*str) || *str == '_' || *str == '-')) {
            if (zasfindval(str, &val, &str)) {
                token1->type = TOKENIMMED;
                token1->val = val;
            } else if (zasgetvalue(str, &val, &str)) {
                token1->type = TOKENIMMED;
                token1->val = val;
            } else {
                name = zasgetsym(str, &str);
                if (name) {
                    token1->type = TOKENADR;
                    token1->data.adr.name = name;
                    token1->data.adr.val = RESOLVE;
                } else {
                    fprintf(stderr, "invalid token %s\n", linebuf);
                    
                    exit(1);
                }
            }
        } else if ((*str) && isdigit(*str)) {
            if (zasgetvalue(str, &val, &str)) {
                token1->type = TOKENIMMED;
                token1->val = val;
            } else {
                fprintf(stderr, "invalid immediate %s\n", str);

                exit(1);
            }
        }
    } else if (*str == '\'') {
        val = zasgetchar(str, &str);
        token1->type = TOKENCHAR;
        token1->data.ch = val;
    } else if (*str == '.') {
        str++;
        size = 0;
        if (!strncmp((char *)str, "quad", 4)) {
            str += 4;
            token1->type = TOKENDATA;
            size = token1->data.size = 8;
        } else if (!strncmp((char *)str, "long", 4)) {
            str += 4;
            token1->type = TOKENDATA;
            size = token1->data.size = 4;
        } else if (!strncmp((char *)str, "byte", 4)) {
            str += 4;
            token1->type = TOKENDATA;
            size = token1->data.size = 1;
        } else if (!strncmp((char *)str, "short", 5)) {
            str += 5;
            token1->type = TOKENDATA;
            size = token1->data.size = 2;
        } else if (!strncmp((char *)str, "globl", 5)) {
            str += 5;
            token1->type = TOKENGLOBL;
        } else if (!strncmp((char *)str, "space", 5)) {
            str += 5;
            token1->type = TOKENSPACE;
        } else if (!strncmp((char *)str, "org", 3)) {
            str += 3;
            token1->type = TOKENORG;
        } else if (!strncmp((char *)str, "align", 5)) {
            str += 5;
            token1->type = TOKENALIGN;
        } else if (!strncmp((char *)str, "asciz", 5)) {
            str += 5;
            token1->type = TOKENASCIZ;
        }
    } else if (*str == '*' || *str == '(') {
        str++;
        token1->type = TOKENINDIR;
        token2 = malloc(sizeof(struct zastoken));
        if (*str == '%') {
            str++;
            val = zasgetreg(str, &str);
            token2->type = TOKENREG;
#if (ZPC)
            token2->data.reg = ZPCREGINDIR | val;
#else
            token2->data.reg = REGINDIR | val;
#endif
            zasqueuetoken(token1);
            token1 = token2;
        } else if (isalpha(*str) || *str == '_') {
            name = zasgetsym(str, &str);
            if (name) {
                token2->type = TOKENSYM;
                token2->data.sym.name = name;
                zasqueuetoken(token1);
                token1 = token2;
            } else {
                fprintf(stderr, "invalid token %s\n", linebuf);

                exit(1);
            }
        } else {
            fprintf(stderr, "invalid token %s\n", linebuf);
            
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
    uint8_t         *valptr = &physmem[adr];
    struct zastoken *retval;

#if (ZPC)
    switch (token->token->type) {
        case ZPCINT64:
            *((int64_t *)valptr) = token->token->data.ui64.i64;
            *retadr = adr + sizeof(int64_t);

            break;
        case ZPCUINT64:
            *((uint64_t *)valptr) = token->token->data.ui64.u64;
            *retadr = adr + sizeof(uint64_t);

            break;
        case ZPCFLOAT:
            *((float *)valptr) = token->token->data.f32;
            *retadr = adr + sizeof(float);

            break;
        case ZPCDOUBLE:
            *((double *)valptr) = token->token->data.f64;
            *retadr = adr + sizeof(double);

            break;
    }
#else
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
    *retadr = adr + token->data.value.size;
#endif
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
zasprocinst(struct zastoken *token, zasmemadr_t adr,
            zasmemadr_t *retadr)
{
#if (ZPC)
    struct zpcopcode *op;
#else
    struct wpmopcode *op;
#endif
    zasmemadr_t       opadr = rounduppow2(adr, 4);
    struct zastoken  *token1 = NULL;
    struct zastoken  *token2 = NULL;
    struct zastoken  *retval = NULL;
    struct zassymrec *sym;
    uint8_t           narg = token->data.inst.narg;
#if (ZPC)
    uint8_t           len = 8;
#else
    uint8_t           len = token->data.inst.op == OPNOP ? 1 : 4;
#endif

#if (ZPC)
    adr = opadr;
#else
    while (adr < opadr) {
        physmem[adr] = OPNOP;
        adr++;
    }
#endif
//    adr = opadr;
#if (ZASDB)
    zasaddline(adr, token->data.inst.data, token->file, token->line);
#endif
#if (ZPC)
    op = (struct zpcopcode *)&physmem[adr];
    op->inst = token->data.inst.op;
#else
    op = (struct wpmopcode *)&physmem[adr];
    op->inst = token->data.inst.op;
    if (op->inst == OPNOP) {
        retval = token->next;
        adr++;
    } else
#endif
    if (!narg) {
        op->arg1t = ARGNONE;
        op->arg2t = ARGNONE;
        op->reg1 = 0;
        op->reg2 = 0;
        retval = token->next;
    } else {
        token1 = token->next;
        zasfreetoken(token);
        if (token1) {
            switch(token1->type) {
                case TOKENVALUE:
                    op->arg1t = ARGIMMED;
#if (ZPC)
                    op->args[0] = token1->token->data.ui64.u64;
#else
                    op->args[0] = token1->data.value.val;
#endif
                    len += sizeof(zasword_t);
                    
                    break;
                case TOKENREG:
                    op->arg1t = ARGREG;
                    op->reg1 = token1->data.reg;
                    
                    break;
                case TOKENSYM:
                    op->arg1t = ARGADR;
                    sym = malloc(sizeof(struct zassymrec));
                    sym->name = (uint8_t *)strdup((char *)token1->data.sym.name);
                    sym->adr = (uintptr_t)&op->args[0];
                    zasqueuesym(sym);
                    len += 4;
                    
                    break;
                case TOKENINDIR:
                    token1 = token1->next;
                    if (token1->type == TOKENREG) {
                        op->arg1t = ARGREG;
                        op->reg1 = token1->data.reg;
                    } else {
                        fprintf(stderr, "indirect addressing requires a register\n");

                        exit(1);
                    }
                    
                    break;
                case TOKENIMMED:
                    op->arg1t = ARGIMMED;
                    op->args[0] = token1->val;
                    len += 4;
                    
                    break;
                case TOKENADR:
                    op->arg1t = ARGIMMED;
                    sym = malloc(sizeof(struct zassymrec));
                    sym->name = (uint8_t *)strdup((char *)token1->data.sym.name);
                    sym->adr = (uintptr_t)&op->args[0];
                    zasqueuesym(sym);
                    len += 4;

                    break;
                case TOKENINDEX:
                    op->arg1t = ARGREG;
                    op->reg1 = token1->data.ndx.reg;
                    op->args[0] = token1->data.ndx.val;
                    len += 4;

                    break;
                default:
                    fprintf(stderr, "invalid argument 1 of type %lx\n", token1->type);
                    printtoken(token1);

                    exit(1);

                    break;
            }
            token2 = token1->next;
            zasfreetoken(token1);
            retval = token2;
        }
        if (narg == 1) {
            op->arg2t = ARGNONE;
#if 0
#if (ZPC)
            op->reg2 = ZPCNREG;
#elif (WPM)
            op->reg2 = NREG;
#endif
#endif
        } else if (narg == 2 && (token2)) {
            switch(token2->type) {
                case TOKENVALUE:
                    op->arg2t = ARGIMMED;
#if (ZPC)
                    if (op->arg1t == ARGREG) {
                        op->args[0] = token2->token->data.ui64.i64;
                    } else {
                        op->args[1] = token2->token->data.ui64.i64;
                    }
#else
                    if (op->arg1t == ARGREG) {
                        op->args[0] = token2->data.value.val;
                    } else {
                        op->args[1] = token2->data.value.val;
                    }
#endif
                    len += sizeof(zasword_t);

                    break;
                case TOKENREG:
                    op->arg2t = ARGREG;
                    op->reg2 = token2->data.reg;
                    
                    break;
                case TOKENSYM:
                    op->arg2t = ARGADR;
                    sym = malloc(sizeof(struct zassymrec));
                    sym->name = (uint8_t *)strdup((char *)token2->data.sym.name);
                    if (op->arg1t == ARGREG) {
                        sym->adr = (uintptr_t)&op->args[0];
                    } else {
                        sym->adr = (uintptr_t)&op->args[1];
                    }
                    zasqueuesym(sym);
                    len += 4;
                    
                    break;
                case TOKENINDIR:
                    token2 = token2->next;
                    if (token2->type == TOKENREG) {
                        op->arg2t = ARGREG;
                        op->reg2 = token2->data.reg;
                    } else {
                        fprintf(stderr, "indirect addressing requires a register\n");

                        exit(1);
                    }
                    
                    break;
                case TOKENIMMED:
                    op->arg2t = ARGIMMED;
                    if (op->arg1t == ARGREG) {
                        op->args[0] = token2->val;
                    } else {
                        op->args[1] = token2->val;
                    }
                    len += 4;
                    
                        break;
                case TOKENADR:
                    op->arg2t = ARGIMMED;
                    sym = malloc(sizeof(struct zassymrec));
                    sym->name = (uint8_t *)strdup((char *)token2->data.sym.name);
                    if (op->arg1t == ARGREG) {
                        sym->adr = (uintptr_t)&op->args[0];
                    } else {
                        sym->adr = (uintptr_t)&op->args[1];
                    }
                    zasqueuesym(sym);
                    len += 4;

                    break;
                case TOKENINDEX:
                    op->arg2t = ARGREG;
                    op->reg2 = token2->data.ndx.reg;
                    if (op->arg1t == ARGREG) {
                        op->args[0] = token2->data.ndx.val;
                    } else {
                        op->args[1] = token2->data.ndx.val;
                    }
                    len += 4;

                    break;
                default:
                    fprintf(stderr, "invalid argument 2 of type %lx\n", token2->type);
                    printtoken(token2);

                    exit(1);

                    break;
            }
            retval = token2->next;
            zasfreetoken(token2);
        }
    }
    op->size = len >> 2;
    *retadr = adr + len;

    return retval;
}

static struct zastoken *
zasprocchar(struct zastoken *token, zasmemadr_t adr,
            zasmemadr_t *retadr)
{
    uint8_t         *valptr = &physmem[adr];
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

    while ((token1) && token1->type == TOKENVALUE) {
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
    while ((token1) && token1->type == TOKENSYM) {
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
    if ((token1) && token1->type == TOKENVALUE) {
#if (ZPC)
        spcadr = token1->token->data.ui64.u64;
#else
        spcadr = token1->data.value.val;
#endif
        token2 = token1->next;
        if ((token2) && token2->type == TOKENVALUE) {
            ptr = &physmem[spcadr];
#if (ZPC)
            val = token2->token->data.ui64.u64;
#else
            val = token2->data.value.val;
#endif
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
    if ((token1) && token1->type == TOKENVALUE) {
        ptr = &physmem[adr];
#if (ZPC)
        orgadr = token1->token->data.ui64.u64;
        val = orgadr;
#else
        orgadr = token1->data.value.val;
        val = token1->data.value.val;
#endif
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
    if ((token1) && token1->type == TOKENVALUE) {
#if (ZPC)
        adr = rounduppow2(adr, token1->token->data.ui64.u64);
#else
        adr = rounduppow2(adr, token1->data.value.val);
#endif
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
    while ((token1) && token1->type == TOKENSTRING) {
        ptr = &physmem[adr];
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
zasinit(struct zasopinfo *opinfotab)
{
    zasopinfotab = opinfotab;
    zasinitop();
    zasinitbuf();
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
            token1 = func(token, adr, &adr);
            if (!token) {

                break;
            }
        } else {
            fprintf(stderr, "stray token of type %lx\n", token->type);
            printtoken(token);

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
        if (sym->adr == RESOLVE) {
            fprintf(stderr, "unresolved symbol %s\n", sym->name);

            exit(1);
        }
        item = zasfindsym(sym->name);
        if (item) {
            if (item->adr == RESOLVE) {
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
    long             buflen = LINELEN;
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
    uint8_t         *str = linebuf;
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
            str = linebuf;
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
                                len, buflen, linebuf);
                        
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
                str = linebuf;
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
                            strbuf);

                    exit(1);
                }
            }
            done = 1;
        } else if (str[0] == ';'
                   || (str[0] == '/' && str[1] == '/')) {
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

