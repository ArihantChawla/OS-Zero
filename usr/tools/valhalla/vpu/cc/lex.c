#include <ctype.h>
#include <wchar.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <limits.h>
#if (ccPRINT)
#include <unistd.h>
#endif
#include <sys/stat.h>
#include <zero/param.h>
#include <zero/cdefs.h>
#include <zero/trix.h>
#include <cc/cc.h>
#include <cc/tune.h>

#define CC_FILE_ERROR (-1)

#define NVALHASH 4096
#define NFILE    1024
#define NLINEBUF 4096

struct cctoken * ccmktype(struct cctoken *token, struct cctoken **nextret,
                          struct cctoken **lastret);

extern struct ccmach   ccmach;
#if (NEWHASH)
extern struct hashstr *qualhash[];
extern struct hashstr *preprochash[];
extern struct hashstr *tunehash[];
extern struct hashstr *atrhash[];
#else
extern struct hashstr  qualhash[];
extern struct hashstr  preprochash[];
extern struct hashstr  atrhash[];
#endif

static long qualflgtab[16] = {
    0,
    CC_EXTERN,         // CC_EXTERN_QUAL
    CC_STATIC,         // CC_STATIC_QUAL
    CC_CONST,          // CC_CONST_QUAL
    CC_VOLATILE        // CC_VOLATILE_QUAL
};

static int  ccreadfile(char *name, int curfile, int doinclude);
static void lexqueuetoken(struct cctoken *token, int curfile);
#if (NEWHASH)
extern void ccaddid(struct hashstr **tab, char *str, long val);
extern long ccfindid(struct hashstr **tab, char *str);
#else
extern void ccaddid(struct hashstr *tab, char *str, long val);
extern long ccfindid(struct hashstr *tab, char *str);
#endif
struct ccstruct * ccgetstruct(struct cctoken *token,
                              struct cctoken **nextret,
                              size_t *sizeret);
struct ccunion * ccgetunion(struct cctoken *token,
                            struct cctoken **nextret,
                            size_t *sizeret);

#define ccisoper(cp)    (optab[(int)(*(cp))])
#define ccistypedef(cp) (!strncmp(cp, "typedef", 7))
#define ccisstruct(cp)  (!strncmp(cp, "struct", 6))
#define ccisunion(cp)   (!strncmp(cp, "union", 5))
#define ccisinline(cp)  (!strncmp(cp, "__inline__", 10))
#define ccisatr(cp)     (!strncmp(cp, "__attribute__", 13))
#define ccispreproc(cp) (*str == '#')
#define ccqualid(cp)    ccfindid(qualhash, cp)
#define ccpreprocid(cp) ccfindid(preprochash, cp)
#define ccatrid(cp)     ccfindid(atrhash, cp)

static char    linebuf[NLINEBUF] ALIGNED(PAGESIZE);
static uint8_t optab[256];
static uint8_t toktab[256];
#if (CCSTAT)
unsigned long  tokcnttab[256];
char          *toknametab[256] =
{
    "CC_NONE",
    "CC_TYPE_TOKEN",
    "CC_TYPEDEF_TOKEN",
    "CC_VAR_TOKEN",
    "CC_STRUCT_TOKEN",
    "CC_UNION_TOKEN",
    "CC_OPER_TOKEN",
    "CC_DOT_TOKEN",
    "CC_INDIR_TOKEN",
    "CC_ASTERISK_TOKEN",
    "CC_COMMA_TOKEN",
    "CC_SEMICOLON_TOKEN",
    "CC_COLON_TOKEN",
    "CC_EXCLAMATION_TOKEN",
    "CC_LEFT_PAREN_TOKEN",
    "CC_RIGHT_PAREN_TOKEN",
    "CC_INDEX_TOKEN",
    "CC_END_INDEX_TOKEN",
    "CC_BLOCK_TOKEN",
    "CC_END_BLOCK_TOKEN",
    "CC_QUOTE_TOKEN",
    "CC_DOUBLE_QUOTE_TOKEN",
    "CC_BACKSLASH_TOKEN",
    "CC_NEWLINE_TOKEN",
    "CC_VALUE_TOKEN",
    "CC_STRING_TOKEN",
    "CC_LITERAL_TOKEN",
    "CC_QUAL_TOKEN",
    "CC_ATR_TOKEN",
    "CC_FUNC_TOKEN",
    "CC_LABEL_TOKEN",
    "CC_ADR_TOKEN",
    "CC_MACRO_TOKEN",
    "CC_PREPROC_TOKEN",
    "CC_CONCAT_TOKEN",
    "CC_STRINGIFY_TOKEN"
};
#endif
long typesztab[32] = {
    0,                  // CC_NONE
    1,                  // CC_CHAR
    1,                  // CC_UCHAR
    2,                  // CC_SHORT
    2,                  // CC_USHORT
    4,                  // CC_INT
    4,                  // CC_UINT
    LONGSIZE,           // CC_LONG
    LONGSIZE,           // CC_ULONG
    LONGLONGSIZE,       // CC_LONGLONG
    LONGLONGSIZE        // CC_ULONGLONG
};
static long typesigntab[32] = {
    0,                  // CC_NONE
    1,                  // CC_CHAR
    0,                  // CC_UCHAR
    1,                  // CC_SHORT
    0,                  // CC_USHORT
    1,                  // CC_INT
    0,                  // CC_UINT
    1,                  // CC_LONG
    0,                  // CC_ULONG
    1,                  // CC_LONGLONG
    0                   // CC_ULONGLONG
};
static long parmlentab[16] = {
    0,                  // CC_NONE
    6,                  // CC_EXTERN_QUAL
    6,                  // CC_STATIC_QUAL
    5,                  // CC_CONST_QUAL
    8,                  // CC_VOLATILE_QUAL
    2,                  // CC_IF_DIR
    4,                  // CC_ELIF_DIR
    4,                  // CC_ELSE_DIR
    5,                  // CC_ENDIF_DIR
    5,                  // CC_IFDEF_DIR
    6,                  // CC_IFNDEF_DIR
    6,                  // CC_DEFINE_DIR
    5                   // CC_UNDEF_DIR
};
static long atrlentab[16] = {
    0,                  // CC_NONE
    6,                  // CC_ATR_PACKED
    7,                  // CC_ATR_ALIGNED
    12,                 // CC_ATR_NORETURN
    10                  // CC_ATR_FORMAT
};
static struct cctokenq *ccfiletokens;
static int               cccurfile;
unsigned int             ccnfiles;
static long              ccoptflags;
#if (CCTOKENCNT)
unsigned long            ntoken;
#endif

static void
ccusage(void)
{
    fprintf(stderr, "usage <options> [file1] .. [fileN]\n");
    fprintf(stderr, "\t-h OR --help\tprint this help message\n");

    return;
}

static void
ccinitoptab(void)
{
    optab['!'] = '!';
    optab['~'] = '~';
    optab['&'] = '&';
    optab['|'] = '|';
    optab['^'] = '^';
    optab['<'] = '<';
    optab['>'] = '>';
    optab['+'] = '+';
    optab['-'] = '-';
    //    optab['*'] = '*';
    optab['/'] = '/';
    optab['%'] = '%';
    optab['='] = '=';
}

static void
ccinittoktab(void)
{
    toktab[';'] = CC_SEMICOLON_TOKEN;
    toktab['.'] = CC_DOT_TOKEN;
    toktab[','] = CC_COMMA_TOKEN;
    toktab[';'] = CC_SEMICOLON_TOKEN;
    toktab['{'] = CC_BLOCK_TOKEN;
    toktab['}'] = CC_END_BLOCK_TOKEN;
    toktab['?'] = CC_EXCLAMATION_TOKEN;
    toktab[':'] = CC_COLON_TOKEN;
    toktab['('] = CC_LEFT_PAREN_TOKEN;
    toktab[')'] = CC_RIGHT_PAREN_TOKEN;
    toktab['['] = CC_INDEX_TOKEN;
    toktab[']'] = CC_END_INDEX_TOKEN;
    toktab['\''] = CC_QUOTE_TOKEN;
    toktab['"'] = CC_DOUBLE_QUOTE_TOKEN;
    toktab['\\'] = CC_BACKSLASH_TOKEN;
    toktab['\n'] = CC_NEWLINE_TOKEN;
}

static void
ccinithash(void)
{
    /* qualifiers */
    ccaddid(qualhash, "extern", CC_EXTERN_QUAL);
    ccaddid(qualhash, "static", CC_STATIC_QUAL);
    ccaddid(qualhash, "const", CC_CONST_QUAL);
    ccaddid(qualhash, "volatile", CC_VOLATILE_QUAL);
    /* preprocessor directives */
    ccaddid(preprochash, "ifdef", CC_IFDEF_DIR);
    ccaddid(preprochash, "elif", CC_ELIF_DIR);
    ccaddid(preprochash, "else", CC_ELSE_DIR);
    ccaddid(preprochash, "endif", CC_ENDIF_DIR);
    ccaddid(preprochash, "ifndef", CC_IFNDEF_DIR);
    ccaddid(preprochash, "if", CC_IF_DIR);
    ccaddid(preprochash, "define", CC_DEFINE_DIR);
    ccaddid(preprochash, "undef", CC_UNDEF_DIR);
    /* optimiser options */
    ccaddid(tunehash, "__inline__", CC_TUNE_INLINE);
    /* compiler attributes */
    ccaddid(atrhash, "packed", CC_ATR_PACKED);
    ccaddid(atrhash, "aligned", CC_ATR_ALIGNED);
    ccaddid(atrhash, "__noreturn__", CC_ATR_NORETURN);
    ccaddid(atrhash, "__format__", CC_ATR_FORMAT);
}

static int
ccinit(int argc,
       char *argv[])
{
    int          l;
    char        *str;

    if (argc == 1) {
        fprintf(stderr, "%s: arguments needed\n", argv[0]);

        return 0;
    }
    ccinitoptab();
    ccinittoktab();
    ccinithash();
    for (l = 1 ; l < argc ; l++) {
        str = argv[l];
        if (*str == '-') {
            if (!strcmp(str, "-h") || !strcmp(str, "--help")) {
                ccusage();

                exit(0);
            } else if (!strcmp(str, "-O")) {
                ccoptflags |= CC_TUNE_ALIGN;
            }
        } else {

            break;
        }
    }
    ccfiletokens = malloc(NFILE * sizeof(struct cctokenq));
    ccnfiles = NFILE;

    return l;
}

static int
ccgettype(char *str, char **retstr)
{
    long type = CC_NONE;

    if (!strncmp(str, "unsigned char", 13)) {
        type = CC_UCHAR;
        str += 13;
    } else if (!strncmp(str, "char", 4)) {
        type = CC_CHAR;
        str += 4;
    } else if (!strncmp(str, "unsigned short", 14)) {
        type = CC_USHORT;
        str += 14;
    } else if (!strncmp(str, "short", 5)) {
        type = CC_SHORT;
        str += 14;
    } else if (!strncmp(str, "unsigned int", 12)) {
        type = CC_UINT;
        str += 12;
    } else if (!strncmp(str, "int", 3)) {
        type = CC_INT;
        str += 3;
    } else if (!strncmp(str, "unsigned long long", 18)) {
        type = CC_ULONGLONG;
        str += 18;
    } else if (!strncmp(str, "long long", 9)) {
        type = CC_LONGLONG;
        str += 9;
    } else if (!strncmp(str, "unsigned", 8)) {
        type = CC_LONGLONG;
        str += 8;
    } else if (!strncmp(str, "signed char", 11)) {
        type = CC_CHAR;
        str += 11;
    } else if (!strncmp(str, "signed short", 12)) {
        type = CC_SHORT;
        str += 12;
    } else if (!strncmp(str, "signed int", 10)) {
        type = CC_INT;
        str += 10;
    } else if (!strncmp(str, "signed long", 11)) {
        type = CC_LONG;
        str += 11;
    } else if (!strncmp(str, "signed long long", 16)) {
        type = CC_LONGLONG;
        str += 16;
    } else if (!strncmp(str, "void", 4)) {
        type = CC_VOID;
        str += 4;
    }
    if (type != CC_NONE) {
        *retstr = str;
    }

    return type;
}

static char *
ccgetstr(char *str, char **retstr)
{
    char   *buf = malloc(32);
    char   *ptr = buf;
    char   *mptr;
    size_t  len = 32;
    size_t  n = 0;

    while (*str != '"') {
        if (n == len) {
            len <<= 1;
            mptr = realloc(buf, len);
            if (!mptr) {
                free(buf);
                fprintf(stderr, "out of memory\n");

                exit(1);
            }
            buf = mptr;
            ptr = &buf[n];
        }
#if 0
        if (*str == '\\') {
            str++;
            switch (*str) {
                case 'b':
                    *ptr++ = '\b';

                    break;
                case 'n':
                    *ptr++ = '\n';

                    break;
                case 't':
                    *ptr++ = '\t';

                    break;
                case 'r':
                    *ptr++ = '\r';

                    break;
                default:
                    fprintf(stderr, "unknown escape sequence in %s\n", buf);

                    exit(1);
            }
            str++;
        } else {
            *ptr++ = *str++;
        }
#endif
        *ptr++ = *str++;
        n++;
    }
    str++;
    if (buf) {
        if (n == len) {
            len <<= 1;
            mptr = realloc(buf, len);
            if (!mptr) {
                free(buf);
                fprintf(stderr, "out of memory\n");
            }
            buf = mptr;
        }
        buf[n] = '\0';
        *retstr = str;
    }

    return buf;
}

static unsigned long
ccgetwliter(char *str, char **retstr)
{
    unsigned long found = 0;
    unsigned long val = 0;

#if (CCDEBUG)
    fprintf(stderr, "getwliter: %s\n", str);
#endif
    if (tolower(*str) == 'x') {
        str++;
        while ((*str) && isxdigit(*str)) {
            val <<= 4;
            val += (isdigit(*str)
                   ? *str - '0'
                   : tolower(*str) - 'a' + 10);
            str++;
        }
        found = 1;
    } else if (tolower(*str) == 'b') {
        str++;
        while (*str == '0' || *str == '1') {
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
    } else {
        val = *((wchar_t *)str);
    }
    if (*str == '\'') {
        str++;
    }
    if (found) {
        *retstr = str;
    }

    return val;
}

static unsigned long
ccgetliter(char *str, char **retstr)
{
    unsigned long found = 0;
    unsigned long val = 0;

#if (CCDEBUG)
    fprintf(stderr, "getliter: %s\n", str);
#endif
    if (tolower(*str) == 'x') {
        str++;
        while ((*str) && isxdigit(*str)) {
            val <<= 4;
            val += (isdigit(*str)
                   ? *str - '0'
                   : tolower(*str) - 'a' + 10);
            str++;
        }
        found = 1;
    } else if (tolower(*str) == 'b') {
        str++;
        while (*str == '0' || *str == '1') {
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
    } else {
        val = *((uint8_t *)str);
    }
    if (*str == '\'') {
        str++;
    }
    if (found) {
        *retstr = str;
    }

    return val;
}

struct cctoken *
ccmktype(struct cctoken *token, struct cctoken **nextret,
         struct cctoken **lastret)
{
    struct cctoken *head = NULL;
    struct cctoken *tail = NULL;
    struct cctoken *last = NULL;
    struct cctoken *tok = malloc(sizeof(struct cctoken));;
    long             parm;
    long             def = 0;

    if (token->type == CC_TYPEDEF_TOKEN) {
        token = token->next;
        def = 1;
    }
    if (!token) {
        if (def) {
            fprintf(stderr, "invalid typedef\n");

            exit(1);
        }
    }
    if (token->type == CC_ATR_TOKEN) {
        token = token->next;
        while (token->type == CC_LEFT_PAREN_TOKEN) {
            token = token->next;
        }

    }
    tok->type = CC_NONE;
    tok->parm = CC_NONE;
    tok->str = NULL;
    tok->data = 0;
    tok->datasz = 0;
    if (token->type == CC_QUAL_TOKEN) {
        parm = token->parm;
        tok->flg |= qualflgtab[parm];
        token = token->next;
        while (token->type == CC_QUAL_TOKEN) {
            parm = token->parm;
            tok->flg |= qualflgtab[parm];
            token = token->next;
        }
    }
    if (token->type == CC_STRUCT_TOKEN) {
        tok->type = CC_STRUCT_TOKEN;
        token = token->next;
        if (token->type == CC_VAR_TOKEN) {
            tok->str = strdup(token->str);
            tok->data = (uintptr_t)ccgetstruct(token, &token, &tok->datasz);
        } else {
            fprintf(stderr, "invalid structure %s\n", token->str);
            free(tok);
            tok = NULL;
        }
    } else if (token->type == CC_UNION_TOKEN) {
        tok->type = CC_UNION_TOKEN;
        token = token->next;
        if (token->type == CC_VAR_TOKEN) {
            tok->str = strdup(token->str);
            tok->data = (uintptr_t)ccgetunion(token, &token, &tok->datasz);
        } else {
            fprintf(stderr, "invalid union %s\n", token->str);
            free(tok);
            tok = NULL;
        }
    } else if (token->type == CC_TYPE_TOKEN) {
        tok->type = CC_TYPE_TOKEN;
        parm = cctoktype(token);
        tok->parm = parm;
        parm = ccvarsz(parm);
        if (parm) {
            tok->datasz = parm;
            tok->str = strdup(token->str);
        } else {
            fprintf(stderr, "invalid type %s\n", token->str);
            free(tok);
            tok = NULL;
        }
    }
    if  (tok) {
        ccqueuetoken(tok, last, &head, &tail);
        *nextret = token;
        *lastret = NULL;
    }

    return tok;
}

struct ccstruct *
ccgetstruct(struct cctoken *token, struct cctoken **nextret, size_t *sizeret)
{
    struct ccstruct *newstruct = NULL;
    struct cctoken  *tok;
    struct cctoken  *last;
    void             *mptr;
    long              parm;
    size_t            msz;
    size_t            sz = 0;
    size_t            nmemb = 8;
    size_t            n = 0;

    if (token->type == CC_BLOCK_TOKEN) {
        newstruct = malloc(sizeof(struct ccstruct));
        newstruct->mtab = malloc(nmemb * sizeof(struct cctoken *));
        newstruct->ofstab = malloc(nmemb * sizeof(size_t));
        token = token->next;
        while ((token) && token->type != CC_END_BLOCK_TOKEN) {
            if (token->type == CC_TYPE_TOKEN) {
                tok = ccmktype(token, &token, &last);
                if (token->type == CC_VAR_TOKEN) {
                    if (n == nmemb) {
                        nmemb <<= 1;
                        mptr = realloc(newstruct->mtab,
                               nmemb * sizeof(struct cctoken *));
                        if (!mptr) {
                            free(newstruct->mtab);
                            fprintf(stderr, "out of memory\n");

                            exit(1);
                        }
                        newstruct->mtab = mptr;
                        mptr = realloc(newstruct->ofstab,
                               nmemb * sizeof(size_t));
                        if (!mptr) {
                            free(newstruct->ofstab);
                            fprintf(stderr, "out of memory\n");

                            exit(1);
                        }
                        newstruct->ofstab = mptr;
                    }
                    tok = malloc(sizeof(struct cctoken));
                    if (tok) {
                        parm = tok->parm;
                        tok->type = CC_VAR_TOKEN;
                        //                        msz = ccvarsz(parm);
                        if (tok->flg & CC_PACKED) {
                            msz = ccvarsz(parm);
                        } else {
                            msz = max(ccvarsz(parm), ccmach.ialn);
                        }
                        tok->datasz = msz;
                        token = token->next;
                        if (token->type == CC_SEMICOLON_TOKEN) {
                            token = token->next;
                            newstruct->mtab[n] = tok;
                            newstruct->ofstab[n] = sz;
                            sz += msz;
                            n++;
                        } else {
                            fprintf(stderr, "unexpected token in struct: %s\n",
                                    token->str);
                            free(tok);
                        }
                    } else {
                        fprintf(stderr, "cannot allocate token\n");

                        exit(1);
                    }
                }
            }
        }
        if (token->type == CC_SEMICOLON_TOKEN) {
            token = token->next;
        }
    }
    if (newstruct) {
        newstruct->nmemb = n;
        *nextret = token;
        *sizeret = sz;
    }

    return newstruct;
}

struct ccunion *
ccgetunion(struct cctoken *token, struct cctoken **nextret, size_t *sizeret)
{
    return NULL;
}

/* TODO: floating-point values */

static struct ccval *
ccgetval(char *str, char **retstr)
{
    char               *cptr = str;
    long                type = CC_NONE;
    unsigned long long  uval = 0;
    unsigned long long  fval = 0;
    long long           tmp = 0;
    long long           mant = 0;
    long long           exp = 0;
    long                found = 0;
    long                flt = 0;
    long                eneg = 0;
    long long           val;
    long                neg = 0;
    struct ccval      *newval = NULL;

#if (CCDEBUG)
    fprintf(stderr, "getval: %s\n", str);
#endif
    if (*cptr == '-') {
        neg = 1;
        cptr++;
    }
    if (cptr[0] == '0' && tolower(cptr[1]) == 'x') {
        cptr += 2;
        while ((*cptr) && isxdigit(*cptr)) {
            uval <<= 4;
            uval += (isdigit(*cptr)
                    ? *cptr - '0'
                    : tolower(*cptr) - 'a' + 10);
            cptr++;
        }
        if ((*cptr) && *cptr == '.') {
            cptr++;
            flt = 1;
            while ((*cptr) && isxdigit(*cptr)) {
                mant >>= 4;
                tmp = (isdigit(*cptr)
                      ? *cptr - '0'
                      : tolower(*cptr) - 'a' + 10);
                mant |= tmp << (LONGLONGSIZE * CHAR_BIT - 5);
                cptr++;
            }
            if (tolower(*cptr) == 'e') {
                if ((cptr[1]) && cptr[1] == '-') {
                    eneg = 1;
                    cptr++;
                }
                while ((*cptr) && isxdigit(*cptr)) {
                    exp <<= 4;
                    exp += (isdigit(*cptr)
                           ? *cptr - '0'
                           : tolower(*cptr) - 'a' + 10);
                    cptr++;
                }
            }
        }
        found = 1;
    } else if (cptr[0] == '0' && tolower(cptr[1]) == 'b') {
        cptr += 2;
        while ((*cptr) && (*cptr == '0' || *cptr == '1')) {
            uval <<= 1;
            uval += *cptr - '0';
            cptr++;
        }
        if (*cptr == '.') {
            cptr++;
            flt = 1;
            while ((*cptr) && *cptr == '0'
                  && (cptr[1]) && cptr[1] == '1') {
                mant >>= 1;
                tmp = *cptr - '0';
                mant |= tmp << (LONGLONGSIZE * CHAR_BIT - 2);
                cptr++;
            }
            if ((*cptr) && tolower(*cptr) == 'e') {
                cptr++;
                if ((*cptr) && *cptr == '-') {
                    eneg = 1;
                    cptr++;
                }
                while ((*cptr) && tolower(*cptr) == '0'
                      && (cptr[1]) && cptr[1] == '1') {
                    exp <<= 1;
                    exp += *cptr - '0';
                    cptr++;
                }
            }
        }
        found = 1;
    } else if (*cptr == '0') {
        cptr++;
        while ((*cptr) && isdigit(*cptr)) {
            if (*cptr > '7') {
                fprintf(stderr, "invalid number in octal constant: %s\n",
                        cptr);

                exit(1);
            }
            uval <<= 3;
            uval += *cptr - '0';
            cptr++;
        }
        if (*cptr == '.') {
            cptr++;
            flt = 1;
            while ((*cptr) && isdigit(*cptr)) {
                if (*cptr > '7') {
                    fprintf(stderr, "invalid number in octal constant: %s\n",
                            cptr);

                    exit(1);
                }
                mant >>= 3;
                tmp = *cptr - '0';
                mant |= tmp << (LONGLONGSIZE * CHAR_BIT - 4);
                cptr++;
            }
            if (tolower(*cptr) == 'e') {
                if ((cptr[1]) && cptr[1] == '-') {
                    eneg = 1;
                    cptr++;
                }
                while ((*cptr) && isxdigit(*cptr)) {
                    exp <<= 3;
                    exp += *cptr - '0';
                    cptr++;
                }
            }
        }
        found = 1;
    } else if (isdigit(*cptr)) {
        while ((*cptr) && isdigit(*cptr)) {
            uval *= 10;
            uval += *cptr - '0';
            cptr++;
        }
        if (*cptr == '.') {
            cptr++;
            flt = 1;
            while ((*cptr) && isdigit(*cptr)) {
                fval /= 10;
                fval += *cptr - '0';
                cptr++;
            }
            if (tolower(*cptr) == 'e') {
                if ((cptr[1]) && cptr[1] == '-') {
                    eneg = 1;
                    cptr++;
                }
                while ((*cptr) && isxdigit(*cptr)) {
                    exp *= 10;
                    exp += *cptr - '0';
                    cptr++;
                }
            }
        }
        found = 1;
    }
    if (found) {
        /* FIXME: range checks for values */
        newval = malloc(sizeof(struct ccval));
        if (flt) {
            if ((*cptr) && tolower(*cptr) == 'l'
               && (cptr[1]) &&tolower(cptr[1] == 'f')) {
                cptr += 2;
                //                mant >>= LONGLONGSIZE * CHAR_BIT - __IEEE754_LONG_DOUBLE_MANT_BITS;
                if (neg) {
                    val = -uval;
                    ldsetsign(newval->fval.ld, 1);
                    ldsetmant(newval->fval.ld, val);
                } else {
                    ldsetmant(newval->fval.ld, uval);
                }
                if (eneg) {
                    val = -exp;
                } else {
                    val = exp;
                }
                ldsetexp(newval->fval.ld, val);
                type = CC_LONG_DOUBLE;
            } else if ((*cptr) && tolower(*cptr) == 'f') {
                cptr++;
                if (neg) {
                    val = -uval;
                    fsetsign(newval->fval.f, 1);
                    fsetmant(newval->fval.f, val);
                } else {
                    fsetmant(newval->fval.f, uval);
                }
                if (eneg) {
                    val = -exp;
                } else {
                    val = exp;
                }
                fsetexp(newval->fval.f, val);
                type = CC_FLOAT;
            } else {
                cptr++;
                if (neg) {
                    val = -uval;
                    dsetsign(newval->fval.d, 1);
                    dsetmant(newval->fval.d, val);
                } else {
                    dsetmant(newval->fval.d, uval);
                }
                if (eneg) {
                    val = -exp;
                } else {
                    val = exp;
                }
                dsetexp(newval->fval.d, val);
                type = CC_DOUBLE;
            }
        } else if (neg) {
            val = -uval;
            if ((*cptr) && tolower(*cptr) == 'l') {
                cptr++;
                if ((*cptr) && tolower(*cptr) == 'l') {
                    cptr++;
                    newval->ival.ll = (long long)val;
                    type = CC_LONGLONG;
                } else {
                    newval->ival.l = (long)val;
                    type = CC_LONG;
                }
            } else {
                newval->ival.i = (int)val;
                type = CC_INT;
            }
        } else {
            if ((*cptr) && tolower(*cptr) == 'u') {
                cptr++;
                if ((*cptr) && tolower(*cptr) == 'l') {
                    cptr++;
                    if ((*cptr) && tolower(*cptr) == 'l') {
                        cptr++;
                        newval->ival.ull = uval;
                        type = CC_ULONGLONG;
                    } else {
                        newval->ival.ul = (unsigned long)uval;
                        type = CC_ULONG;
                    }
                } else {
                    newval->ival.ui = (unsigned int)uval;
                    type = CC_UINT;
                }
            } else {
                newval->ival.i = (int)uval;
                type = CC_INT;
            }
        }
        newval->type = type;
        newval->sz = ccvarsz(type);
        if (*cptr == '\'') {
            cptr++;
        }
        *retstr = cptr;
    }

    return newval;
}

static struct cctoken *
ccgettoken(char *str, char **retstr, int curfile)
{
    long             len = 0;
    long             n;
    long             type;
    long             parm;
    //    long             atr;
    char            *ptr;
    void            *mptr;
    struct cctoken *tok = malloc(sizeof(struct cctoken));
    struct ccval   *val;

#if (CCDEBUG)
    fprintf(stderr, "gettoken: %s\n", str);
#endif
    tok->type = CC_NONE;
    tok->parm = CC_NONE;
    tok->str = NULL;
    tok->data = 0;
    if ((type = toktab[(int)(*str)])) {
        tok->type = type;
        tok->str = malloc(2);
        tok->str[0] = *str;
        tok->str[1] = '\0';
        str++;
    } else if (*str == '"') {
        str++;
        tok->type = CC_STRING_TOKEN;
        tok->str = ccgetstr(str, &str);
    } else if (*str == '*') {
        tok->type = CC_ASTERISK_TOKEN;
        tok->str = strdup("*");
        str++;
    } else if (*str == ',') {
        str++;
        tok->type = CC_COMMA_TOKEN;
        tok->data = CC_NO_ADR;
    } else if (ccisoper(str)) {
        tok->type = CC_OPER_TOKEN;
        len = 8;
        n ^= n;
        tok->str = malloc(len);
        ptr = tok->str;
        while (ccisoper(str)) {
            if (n == len) {
                len <<= 1;
                mptr = realloc(tok->str, len);
                if (!mptr) {
                    free(tok->str);
                    fprintf(stderr, "out of memory\n");

                    exit(1);
                }
                tok->str = mptr;
                ptr = &tok->str[n];
            }
            *ptr++ = *str++;
            n++;
        }
        if (n == len) {
            len <<= 1;
            mptr = realloc(tok->str, len);
            if (!mptr) {
                free(tok->str);
                fprintf(stderr, "out of memory\n");

                exit(1);
            }
            tok->str = mptr;
            ptr = &tok->str[n];
        }
        *ptr = '\0';
    } else if (*str == '#') {
        str++;
        while (isspace(*str)) {
            str++;
        }
        parm = ccpreprocid(str);
        if (parm != CC_NONE) {
            len = parmlentab[parm];
            str += len;
        }
        if (parm == CC_DEFINE_DIR) {
            tok->type = CC_MACRO_TOKEN;
            tok->parm = parm;
        } else if (parm != CC_NONE) {
            tok->type = CC_PREPROC_TOKEN;
            tok->parm = parm;
            str += len;
        } else if (*str == '#') {
            str++;
            tok->type = CC_STRINGIFY_TOKEN;
        } else {
            str++;
            tok->type = CC_CONCAT_TOKEN;
        }
    } else if (*str == 'L' && str[1] == '\'') {
        str += 2;
        tok->type = CC_LITERAL_TOKEN;
        tok->parm = CC_WCHAR;
        tok->data = ccgetwliter(str, &str);
    } else if (*str == '\'') {
        str++;
        tok->type = CC_LITERAL_TOKEN;
        tok->data = ccgetliter(str, &str);
    } else if (*str == '-' && str[1] == '>') {
        tok->type = CC_INDIR_TOKEN;
        tok->str = strndup(str, 2);
        str += 2;
    } else if (ccistypedef(str)) {
        tok->type = CC_TYPEDEF_TOKEN;
        str += 7;
    } else if (ccisatr(str)) {
        tok->type = CC_ATR_TOKEN;
        str += 13;
        while (isspace(*str)) {
            str++;
        }
    } else if ((parm = ccqualid(str))) {
        len = parmlentab[parm];
        tok->type = CC_QUAL_TOKEN;
        tok->parm = parm;
        str += len;
    } else if (ccisstruct(str)) {
        tok->type = CC_STRUCT_TOKEN;
        str += 6;
    } else if (ccisunion(str)) {
        tok->type = CC_UNION_TOKEN;
        str += 5;
    } else if (isalpha(*str) || *str == '_') {
        ptr = str;
        while (isalnum(*str) || *str == '_') {
            str++;
        }
        if (*str == ':') {
            *str = '\0';
            tok->type = CC_LABEL_TOKEN;
            tok->str = strndup(ptr, str - ptr);
            tok->data = CC_NO_ADR;
            str++;
        } else {
            while (isspace(*str)) {
                str++;
            }
            tok->str = strndup(ptr, str - ptr);
            if (*str == '(') {
                tok->type = CC_FUNC_TOKEN;
                tok->data = CC_NO_ADR;
            } else if ((parm = ccgettype(ptr, &str))) {
                tok->type = CC_TYPE_TOKEN;
                tok->parm = parm;
            } else {
                tok->type = CC_VAR_TOKEN;
                tok->data = CC_NO_ADR;
            }
        }
    } else if (isxdigit(*str)) {
        ptr = str;
        val = ccgetval(str, &str);
        if (val) {
            tok->type = CC_VALUE_TOKEN;
            tok->str = strndup(ptr, str - ptr);
            tok->data = (uintptr_t)val;
        }
    } else {
        free(tok);
        tok = NULL;
    }
    if (tok) {
#if (CCSTAT)
        tokcnttab[tok->type]++;
#endif
#if (CCTOKENCNT)
        ntoken++;
#endif
        while (isspace(*str)) {
            str++;
        }
        *retstr = str;
    }

    return tok;
}

static int
ccgetinclude(char *str, char **retstr, int curfile)
{
    char  path[] = "/include/:/usr/include/";
    char  name[PATH_MAX + 1] = { '\0' };
    int   ret = CC_FILE_ERROR;
    char *ptr = name;
    char *cp;

#if (CCDEBUG)
    fprintf(stderr, "getinclude: %s\n", str);
#endif
    if (!strncmp((char *)str, "#include", 8)) {
        str += 8;
        while ((*str) && isspace(*str)) {
            str++;
        }
        if (*str == '"') {
            str++;
            while ((*str) && *str != '"') {
                *ptr++ = *str++;
            }
            if (*str == '"') {
                *ptr = '\0';
                ret = ccreadfile(name, curfile, 1);
            } else {
                fprintf(stderr, "invalid #include directive %s\n",
                        name);

                exit(1);
            }
        } else if (*str == '<') {
            str++;
            ptr = str;
            while ((*str) && *str != '>') {
                str++;
            }
            if (*str == '>') {
                *str = '\0';
                strcat(name, strtok(path, ":"));
                strcat(name, ptr);
                while (ret == CC_FILE_ERROR) {
                    ret = ccreadfile(name, curfile, 1);
                    name[0] = '\0';
                    cp = strtok(NULL, ":");
                    if (cp) {
                        strcat(name, cp);
                        strcat(name, ptr);
                    } else {

                        return CC_FILE_ERROR;
                    }
                }
            } else {
                fprintf(stderr, "invalid #include directive %s\n",
                        name);

                exit(1);
            }
        }
    }
    if (ret != CC_FILE_ERROR) {
        *retstr = str;
    }

    return ret;
}

static void
lexqueuetoken(struct cctoken *token, int curfile)
{
    struct cctokenq *queue = &ccfiletokens[curfile];
    struct cctoken  *head = queue->head;
    struct cctoken  *tail = queue->tail;

#if (CCPRINT) && 0
    fprintf(stderr, "QUEUE\n");
    fprintf(stderr, "-----\n");
    ccprinttoken(token);
#endif
    if (!head) {
        token->prev = NULL;
        queue->head = token;
        queue->tail = NULL;
    } else if (tail) {
        token->prev = tail;
        tail->next = token;
        queue->tail = token;
    } else {
        head->next = token;
        token->prev = head;
        queue->tail = token;
    }

    return;
}

static int
ccreadfile(char *name, int curfile, int doinclude)
{
    long              buflen = NLINEBUF;
    FILE             *fp = fopen(name, "r");
    long              eof = 0;
    long              nl = 0;
    struct cctoken  *tok;
    char             *str = linebuf;
    char             *lim = NULL;
    long              loop = 1;
    int               ch;
    int               tmp;
    long              comm = 0;
    long              done = 1;
    long              len = 0;
#if (CCDB)
    unsigned long     line = 0;
#endif

    if (!fp) {

        return CC_FILE_ERROR;
    }
    while (loop) {
        if (done) {
            if (nl) {
                tok = malloc(sizeof(struct cctoken));
                tok->type = CC_NEWLINE_TOKEN;
                tok->parm = CC_NONE;
                //                tok->str = NULL;
                tok->str = malloc(3);
                tok->str[0] = '\\';
                tok->str[1] = 'n';
                tok->str[2] = '\0';
                tok->data = 0;
#if (CCSTAT)
                tokcnttab[CC_NEWLINE_TOKEN]++;
#endif
                lexqueuetoken(tok, curfile);
                nl = 0;
            }
            if (eof) {
                loop = 0;
                //                done = 0;

                break;
            }
            str = linebuf;
            done = 0;
            len = 0;
            ch = fgetc(fp);
            if (ch == EOF) {
                loop = 0;

                break;
            } else {
#if (CCDB)
                line++;
#endif
                while (ch != EOF && ch != '\n') {
                    if (ch == '\\') {
                        ch = fgetc(fp);
                        if (ch != EOF && !isspace(ch)) {
                            *str++ = '\\';
                            len++;
                            if (len == buflen) {
                                fprintf(stderr, "overlong line\n");

                                exit(1);
                            }
                            switch (ch) {
                                case 'b':
                                case 'n':
                                case 't':
                                case 'r':
                                    *str++ = ch;
                                    len++;
                                    if (len == buflen) {
                                        fprintf(stderr, "overlong line\n");

                                        exit(1);
                                    }

                                    break;
                                default:
                                    fprintf(stderr, "unknown escape sequence\n");

                                    exit(1);
                            }
                        } else {
                            eof = (ch == EOF);
                            if (!eof && isspace(ch)) {
                                while (!eof && isspace(ch)) {
                                    ch = fgetc(fp);
                                    eof = (ch == EOF);
                                }

                                continue;
                            }
                        }
                        ch = fgetc(fp);
                    } else {
                        *str++ = ch;
                        len++;
                        if (len == buflen) {
                            fprintf(stderr, "overlong line\n");

                            exit(1);
                        }
                        ch = fgetc(fp);
                        eof = (ch == EOF);
                    }
                }
                *str = '\0';
                str = linebuf;
                lim = str + len;
                while ((*str) && isspace(*str)) {
                    str++;
                }
                if (str > lim) {
                    done = 1;
                }
                if (ch == '\n') {
                    nl = 1;
                }
                //                fprintf(stderr, "BUF: %s\n", str);
            }
        } else if (doinclude) {
            tmp = ccgetinclude(str, &str, curfile);
            if (tmp != CC_FILE_ERROR) {
                curfile = tmp;
                done = 1;
            } else if (str[0] == '/' && str[1] == '/') {
                /* comment start */
                done = 1;
            } else if (str[0] == '/' && str[1] == '*') {
                /* comment */
                comm = 1;
                while (comm) {
                    ch = fgetc(fp);
                    if (ch == EOF) {
                        loop = 0;

                        break;
#if (CCDB)
                    } else if (ch == '\n') {
                        line++;
#endif
                    } else if (ch == '*') {
                        ch = fgetc(fp);
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
            } else if (*str) {
                tok = ccgettoken(str, &str, curfile);
                if (tok) {
#if (CCDB)
                    tok->fname = strdup(name);
                    tok->line = line;
#endif
                    lexqueuetoken(tok, curfile);
                }
                if (str >= lim) {
                    done = 1;
                }
            } else {
                done = 1;
            }
        } else if (!strncmp((char *)str, "#include", 8)) {
            tok = malloc(sizeof(struct cctoken));
            tok->type = CC_INCLUDE_TOKEN;
            tok->parm = CC_NONE;
            tok->str = strndup(str, 8);
            tok->data = 0;
        } else if (!*str) {
            done = 1;
        }
    }
    fclose(fp);

    return curfile;
}

struct ccinput *
cclex(int argc,
      char *argv[])
{
    struct ccinput *input = NULL;
    int              arg;
    long             l;

    arg = ccinit(argc, argv);
    if (!arg) {

        exit(1);
    }
    for (l = arg; l < argc ; l++) {
        cccurfile = ccreadfile(argv[l], cccurfile, 1);
        if (cccurfile == CC_FILE_ERROR) {

            exit(1);
        }
    }
    input = malloc(sizeof(struct ccinput));
    if (input) {
        input->nq = cccurfile + 1;
        input->qptr = &ccfiletokens;
    }

    return input;
}

