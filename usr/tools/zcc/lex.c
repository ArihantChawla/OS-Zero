#include <ctype.h>
#include <wchar.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <limits.h>
#if (ZCCPRINT)
#include <unistd.h>
#endif
#include <sys/stat.h>
#include <zero/param.h>
#include <zero/cdecl.h>
#include <zcc/zcc.h>

#define ZCC_FILE_ERROR (-1)

#define NVALHASH 4096
#define NFILE  1024
#define NLINEBUF 4096

#if (NEWHASH)
extern struct hashstr *qualhash[];
extern struct hashstr *preprochash[];
extern struct hashstr *atrhash[];
#else
extern struct hashstr qualhash[];
extern struct hashstr preprochash[];
extern struct hashstr atrhash[];
#endif

static int  zccreadfile(char *name, int curfile);
static void zppqueuetoken(struct zpptoken *token, int curfile);
#if (NEWHASH)
extern void zccaddid(struct hashstr **tab, char *str, long val);
extern long zccfindid(struct hashstr **tab, char *str);
#else
extern void zccaddid(struct hashstr *tab, char *str, long val);
extern long zccfindid(struct hashstr *tab, char *str);
#endif

#define zccisoper(cp)    (opertab[(int)(*(cp))])
#define zccistypedef(cp) (!strncmp(cp, "typedef", 7))
#define zccisstruct(cp)  (!strncmp(cp, "struct", 6))
#define zccisunion(cp)   (!strncmp(cp, "union", 5))
#define zccisatr(cp)     (!strncmp(cp, "__attribute__", 13))
#define zccispreproc(cp) (*str == '#')
#define zccqualid(cp)    zccfindid(qualhash, cp)
#define zccpreprocid(cp) zccfindid(preprochash, cp)
#define zccatrid(cp)     zccfindid(atrhash, cp)
#if 0
#define zccqualid(cp)                                                   \
    ((!strncmp(cp, "extern", 6))                                        \
     ? ZCC_EXTERN_QUAL                                                  \
     : (!strncmp(cp, "static", 6)                                       \
        ? ZCC_STATIC_QUAL                                               \
        : (!strncmp(cp, "const", 5)                                     \
           ? ZCC_CONST_QUAL                                             \
           : (!strncmp(cp, "volatile", 8)                               \
              ? ZCC_VOLATILE_QUAL                                       \
              : ZCC_NONE))))
#define zccpreprocid(cp)                                                \
    ((!strncmp(cp, "ifdef", 5))                                         \
     ? ZPP_IFDEF_DIR                                                    \
     : (!strncmp(cp, "elif", 4)                                         \
        ? ZPP_ELIF_DIR                                                  \
        : (!strncmp(cp, "else", 4)                                      \
           ? ZPP_ELSE_DIR                                               \
           : (!strncmp(cp, "endif", 5)                                  \
              ? ZPP_ENDIF_DIR                                           \
              : (!strncmp(cp, "ifndef", 6)                              \
                 ? ZPP_IFNDEF_DIR                                       \
                 : (!strncmp(cp, "if", 2)                               \
                    ? ZPP_IF_DIR                                        \
                    : (!strncmp(cp, "define", 6)                        \
                       ? ZPP_DEFINE_DIR                                 \
                       : (!strncmp(cp, "undef", 5)                      \
                          ? ZPP_UNDEF_DIR))))))))                       \
#define zccatrid(cp)                                                    \
    ((!strncmp(cp, "packed", 6))                                        \
     ? ZCC_ATR_PACKED                                                   \
     : (!strncmp(cp, "aligned", 7)                                      \
        ? ZCC_ATR_ALIGNED                                               \
        : (!strncmp(cp, "__noreturn__", 12)                             \
           ? ZCC_ATR_NORETURN                                           \
           : (!strncmp(cp, "__format__", 10)                            \
              ? ZCC_ATR_FORMAT                                          \
              : ZCC_NONE))))
//#define zccisagr(t)      ((t) == ZCC_STRUCT || (t) == ZCC_UNION)
#endif

#define zccsetival(vp, t, adr)                                          \
    (typesigntab[(t) & 0x1f]                                            \
     ? ((vp)->ival.ll = *((long long *)(adr)))                          \
     : ((vp)->ival.ull = *((unsigned long long *)(adr))))
#define zccsetfval(vp, t, adr)                                          \
    do {                                                                \
        if ((t) == ZCC_FLOAT) {                                         \
            (vp)->fval.f = *(float *)(adr);                             \
        } else if ((t) == ZCC_DOUBLE) {                                 \
            (vp)->fval.d = *(double *)(adr);                            \
        } else if ((t) == ZCC_LONG_DOUBLE) {                            \
            (vp)->fval.d = *(long double *)(adr);                       \
        }                                                               \
    } while (0)

static char    linebuf[NLINEBUF] ALIGNED(PAGESIZE);
static uint8_t opertab[256];
static uint8_t toktab[256];
long           typesztab[32] = {
    0,                  // ZCC_NONE
    1,                  // ZCC_CHAR
    1,                  // ZCC_UCHAR
    2,                  // ZCC_SHORT
    2,                  // ZCC_USHORT
    4,                  // ZCC_INT
    4,                  // ZCC_UINT
    LONGSIZE,           // ZCC_LONG
    LONGSIZE,           // ZCC_ULONG
    LONGLONGSIZE,       // ZCC_LONGLONG
    LONGLONGSIZE        // ZCC_ULONGLONG
};
static long typesigntab[32] = {
    0,                  // ZCC_NONE
    1,                  // ZCC_CHAR
    0,                  // ZCC_UCHAR
    1,                  // ZCC_SHORT
    0,                  // ZCC_USHORT
    1,                  // ZCC_INT
    0,                  // ZCC_UINT
    1,                  // ZCC_LONG
    0,                  // ZCC_ULONG
    1,                  // ZCC_LONGLONG
    0                   // ZCC_ULONGLONG
};
static long parmlentab[16] = {
    0,                  // ZCC_NONE
    6,                  // ZCC_EXTERN_QUAL
    6,                  // ZCC_STATIC_QUAL
    5,                  // ZCC_CONST_QUAL
    8,                  // ZCC_VOLATILE_QUAL
    2,                  // ZCC_IF_DIR
    4,                  // ZCC_ELIF_DIR
    4,                  // ZCC_ELSE_DIR
    5,                  // ZCC_ENDIF_DIR
    5,                  // ZCC_IFDEF_DIR
    6,                  // ZCC_IFNDEF_DIR
    6,                  // ZCC_DEFINE_DIR
    5                   // ZCC_UNDEF_DIR
};
static long atrlentab[16] = {
    0,                  // ZCC_NONE
    6,                  // ZCC_ATR_PACKED
    7,                  // ZCC_ATR_ALIGNED
    12,                 // ZCC_ATR_NORETURN
    10                  // ZCC_ATR_FORMAT
};
static struct zpptokenq *zccfiletokens;
static int               zcccurfile;
unsigned int             zccnfiles;
static long              zccoptflags;
#if (ZPPTOKENCNT)
unsigned long            ntoken;
#endif

static void
zccusage(void)
{
    fprintf(stderr, "usage <options> [file1] .. [fileN]\n");
    fprintf(stderr, "\t-h OR --help\tprint this help message\n");

    return;
}

static void
zccinitopertab(void)
{
    opertab['!'] = '!';
    opertab['~'] = '~';
    opertab['&'] = '&';
    opertab['|'] = '|';
    opertab['^'] = '^';
    opertab['<'] = '<';
    opertab['>'] = '>';
    opertab['+'] = '+';
    opertab['-'] = '-';
    opertab['*'] = '*';
    opertab['/'] = '/';
    opertab['%'] = '%';
    opertab['='] = '=';
}

static void
zccinittoktab(void)
{
    toktab[';'] = ZPP_SEMICOLON_TOKEN;
    toktab['.'] = ZPP_DOT_TOKEN;
    toktab[','] = ZPP_COMMA_TOKEN;
    toktab[';'] = ZPP_SEMICOLON_TOKEN;
    toktab['{'] = ZPP_BLOCK_TOKEN;
    toktab['}'] = ZPP_END_BLOCK_TOKEN;
    toktab['?'] = ZPP_EXCLAMATION_TOKEN;
    toktab[':'] = ZPP_COLON_TOKEN;
    toktab['('] = ZPP_LEFT_PAREN_TOKEN;
    toktab[')'] = ZPP_RIGHT_PAREN_TOKEN;
    toktab['['] = ZPP_INDEX_TOKEN;
    toktab[']'] = ZPP_END_INDEX_TOKEN;
    toktab['\''] = ZPP_QUOTE_TOKEN;
    toktab['"'] = ZPP_DOUBLE_QUOTE_TOKEN;
    toktab['\\'] = ZPP_BACKSLASH_TOKEN;
    toktab['\n'] = ZPP_NEWLINE_TOKEN;
}

static void
zccinithash(void)
{
    /* qualifiers */
    zccaddid(qualhash, "extern", ZCC_EXTERN_QUAL);
    zccaddid(qualhash, "static", ZCC_STATIC_QUAL);
    zccaddid(qualhash, "const", ZCC_CONST_QUAL);
    zccaddid(qualhash, "volatile", ZCC_VOLATILE_QUAL);
    /* preprocessor directives */
    zccaddid(preprochash, "ifdef", ZPP_IFDEF_DIR);
    zccaddid(preprochash, "elif", ZPP_ELIF_DIR);
    zccaddid(preprochash, "else", ZPP_ELSE_DIR);
    zccaddid(preprochash, "endif", ZPP_ENDIF_DIR);
    zccaddid(preprochash, "ifndef", ZPP_IFNDEF_DIR);
    zccaddid(preprochash, "if", ZPP_IF_DIR);
    zccaddid(preprochash, "define", ZPP_DEFINE_DIR);
    zccaddid(preprochash, "undef", ZPP_UNDEF_DIR);
    /* compiler attributes */
    zccaddid(atrhash, "packed", ZCC_ATR_PACKED);
    zccaddid(atrhash, "aligned", ZCC_ATR_ALIGNED);
    zccaddid(atrhash, "__noreturn__", ZCC_ATR_NORETURN);
    zccaddid(atrhash, "__format__", ZCC_ATR_FORMAT);
}

static int
zccinit(int argc,
           char *argv[])
{
    int          l;
    char        *str;

    if (argc == 1) {
        fprintf(stderr, "%s: arguments needed\n", argv[0]);

        return 0;
    }
    zccinitopertab();
    zccinittoktab();
    zccinithash();
    for (l = 1 ; l < argc ; l++) {
        str = argv[l];
        if (*str == '-') {
            if (!strcmp(str, "-h") || !strcmp(str, "--help")) {
                zccusage();
                
                exit(0);
            } else if (!strcmp(str, "-O")) {
                zccoptflags |= ZCC_OPT_ALIGN;
            }
        } else {

            break;
        }
    }
    zccfiletokens = malloc(NFILE * sizeof(struct zpptokenq));
    zccnfiles = NFILE;

    return l;
}

static int
zppgettype(char *str, char **retstr)
{
    long type = ZCC_NONE;

    if (!strncmp(str, "unsigned char", 13)) {
        type = ZCC_UCHAR;
        str += 13;
    } else if (!strncmp(str, "char", 4)) {
        type = ZCC_CHAR;
        str += 4;
    } else if (!strncmp(str, "unsigned short", 14)) {
        type = ZCC_USHORT;
        str += 14;
    } else if (!strncmp(str, "short", 5)) {
        type = ZCC_SHORT;
        str += 14;
    } else if (!strncmp(str, "unsigned int", 12)) {
        type = ZCC_UINT;
        str += 12;
    } else if (!strncmp(str, "int", 3)) {
        type = ZCC_INT;
        str += 3;
    } else if (!strncmp(str, "unsigned long long", 18)) {
        type = ZCC_ULONGLONG;
        str += 18;
    } else if (!strncmp(str, "long long", 9)) {
        type = ZCC_LONGLONG;
        str += 9;
    } else if (!strncmp(str, "unsigned", 8)) {
        type = ZCC_LONGLONG;
        str += 8;
    } else if (!strncmp(str, "signed char", 11)) {
        type = ZCC_CHAR;
        str += 11;
    } else if (!strncmp(str, "signed short", 12)) {
        type = ZCC_SHORT;
        str += 12;
    } else if (!strncmp(str, "signed int", 10)) {
        type = ZCC_INT;
        str += 10;
    } else if (!strncmp(str, "signed long", 11)) {
        type = ZCC_LONG;
        str += 11;
    } else if (!strncmp(str, "signed long long", 16)) {
        type = ZCC_LONGLONG;
        str += 16;
    } else if (!strncmp(str, "void", 4)) {
        type = ZCC_VOID;
        str += 4;
    }
    if (type != ZCC_NONE) {
        *retstr = str;
    }

    return type;
}

static char *
zppgetstr(char *str, char **retstr)
{
    char   *buf = malloc(32);
    char   *ptr = buf;
    size_t  len = 32;
    size_t  n = 0;

    while (*str != '"') {
        if (n == len) {
            len <<= 1;
            buf = realloc(buf, len);
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
            buf = realloc(buf, len);
        }
        buf[n] = '\0';
        *retstr = str;
    }

    return buf;
}

static unsigned long
zppgetwliter(char *str, char **retstr)
{
    unsigned long found = 0;
    unsigned long val = 0;

#if (ZCCDEBUG)
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
zppgetliter(char *str, char **retstr)
{
    unsigned long found = 0;
    unsigned long val = 0;

#if (ZCCDEBUG)
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

/* TODO: floating-point values */
static struct zccval *
zccgetval(char *str, char **retstr)
{
    long                type = ZCC_NONE;
    unsigned long long  uval = 0;
    long                found = 0;
    long long           val;
    long                neg = 0;
    struct zccval      *newval = NULL;

#if (ZCCDEBUG)
    fprintf(stderr, "getval: %s\n", str);
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
        type = ZCC_INT;
        found = 1;
    } else if (str[0] == '0' && tolower(str[1]) == 'b') {
        str += 2;
        while ((*str) && (*str == '0' || *str == '1')) {
            uval <<= 1;
            uval += *str - '0';
            str++;
        }
        type = ZCC_INT;
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
        type = ZCC_INT;
        found = 1;
    } else if (isdigit(*str)) {
        while ((*str) && isdigit(*str)) {
            uval *= 10;
            uval += *str - '0';
            str++;
        }
        type = ZCC_INT;
        found = 1;
    }
    if (found) {
        newval = malloc(sizeof(struct zccval));
        if (neg) {
            val = -uval;
            if (tolower(*str) == 'l') {
                str++;
                if (tolower(*str) == 'l') {
                    type = ZCC_LONGLONG;
                } else {
                    type = ZCC_LONG;
                }
            }
        } else {
            if (tolower(*str) == 'u') {
                str++;
                if (tolower(*str) == 'l') {
                    str++;
                    if (tolower(*str) == 'l') {
                        type = ZCC_ULONGLONG;
                    } else {
                        type = ZCC_ULONG;
                    }
                } else {
                    type = ZCC_UINT;
                }
            }
        }
        newval->type = type;
        newval->sz = zccvarsz(type);
        if (neg) {
            zccsetival(newval, type, &val);
        } else {
            zccsetival(newval, type, &uval);
        }
        while (*str == '\'') {
            str++;
        }
        *retstr = str;
    }

    return newval;
}

static struct zpptoken *
zccgettoken(char *str, char **retstr, int curfile)
{
    long             len = 0;
    long             ndx;
    long             type;
    long             parm;
    long             atr;
    char            *ptr;
    struct zpptoken *tok = malloc(sizeof(struct zpptoken));
    struct zccval   *val;

#if (ZCCDEBUG)
    fprintf(stderr, "gettoken: %s\n", str);
#endif
    tok->type = ZCC_NONE;
    tok->parm = ZCC_NONE;
    tok->str = NULL;
    tok->data = 0;
    if (*str == '"') {
        str++;
        tok->type = ZPP_STRING_TOKEN;
        tok->str = zppgetstr(str, &str);
    } else if (*str == 'L' && str[1] == '\'') {
        str += 2;
        tok->type = ZPP_LITERAL_TOKEN;
        tok->parm = ZCC_WCHAR;
        tok->data = zppgetwliter(str, &str);
    } else if (*str == '\'') {
        str++;
        tok->type = ZPP_LITERAL_TOKEN;
        tok->data = zppgetliter(str, &str);
    } else if (*str == '-' && str[1] == '>') {
        tok->type = ZPP_INDIR_TOKEN;
        tok->str = strndup(str, 2);
        str += 2;
    } else if (*str == '*') {
        tok->type = ZPP_ASTERISK_TOKEN;
        tok->str = strdup("*");
        str++;
    } else if (zccisoper(str)) {
        tok->type = ZPP_OPER_TOKEN;
        len = 8;
        ndx ^= ndx;
        tok->str = malloc(len);
        ptr = tok->str;
        while (zccisoper(str)) {
            if (ndx == len) {
                len <<= 1;
                tok->str = realloc(tok->str, len);
                ptr = &tok->str[ndx];
            }
            *ptr++ = *str++;
            ndx++;
        }
        *ptr = '\0';
    } else if ((type = toktab[(int)(*str)])) {
        tok->type = type;
        str++;
    } else if (*str == '#') {
        str++;
        while (isspace(*str)) {
            str++;
        }
        parm = zccpreprocid(str);
        fprintf(stderr, "found: %lx\n", parm);
        if (parm != ZCC_NONE) {
            len = parmlentab[parm];
            str += len;
        }
        if (parm == ZPP_DEFINE_DIR) {
            tok->type = ZPP_MACRO_TOKEN;
            tok->parm = parm;
        } else if (parm != ZCC_NONE) {
            tok->type = ZPP_PREPROC_TOKEN;
            tok->parm = parm;
            str += len;
        } else if (*str == '#') {
            str++;
            tok->type = ZPP_STRINGIFY_TOKEN;
        } else {
            str++;
            tok->type = ZPP_CONCAT_TOKEN;
        }
    } else if (zccistypedef(str)) {
        tok->type = ZPP_TYPEDEF_TOKEN;
        str += 7;
    } else if (zccisatr(str)) {
        tok->type = ZPP_ATR_TOKEN;
        str += 13;
        while (isspace(*str)) {
            str++;
        }
#if 0
        if ((atr = zccatrid(str)) != ZCC_NONE) {
            len = atrlentab[atr];
            str += len;
        } else {
            fprintf(stderr, "invalid attribute %s\n", str);
        }
#endif
    } else if ((parm = zccqualid(str))) {
        len = parmlentab[parm];
        tok->type = ZPP_QUAL_TOKEN;
        tok->parm = parm;
        str += len;
    } else if (zccisstruct(str)) {
        tok->type = ZPP_STRUCT_TOKEN;
        str += 6;
    } else if (zccisunion(str)) {
        tok->type = ZPP_UNION_TOKEN;
        str += 5;
    } else if (isalpha(*str) || *str == '_') {
        ptr = str;
        while (isalnum(*str) || *str == '_') {
            str++;
        }
        if (*str == ':') {
            *str = '\0';
            tok->type = ZPP_LABEL_TOKEN;
            tok->str = strndup(ptr, str - ptr);
            tok->data = ZCC_NO_ADR;
            str++;
        } else {
            while (isspace(*str)) {
                str++;
            }
            tok->str = strndup(ptr, str - ptr);
            if (*str == '(') {
                tok->type = ZPP_FUNC_TOKEN;
                tok->data = ZCC_NO_ADR;
            } else if ((parm = zppgettype(ptr, &str))) {
                tok->type = ZPP_TYPE_TOKEN;
                tok->parm = parm;
            } else {
                tok->type = ZPP_VAR_TOKEN;
                tok->data = ZCC_NO_ADR;
            }
        }
    } else if (isxdigit(*str)) {
        ptr = str;
        val = zccgetval(str, &str);
        if (val) {
            tok->type = ZPP_VALUE_TOKEN;
            tok->str = strndup(ptr, str - ptr);
            tok->data = (uintptr_t)val;
        }
    } else if (*str == ',') {
        str++;
        tok->type = ZPP_COMMA_TOKEN;
        tok->data = ZCC_NO_ADR;
    } else {
        free(tok);
        tok = NULL;
    }
    if (tok) {
#if (ZPPTOKENCNT)
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
zccgetinclude(char *str, char **retstr, int curfile)
{    
    char  path[] = "/include/:/usr/include/";
    char  name[PATH_MAX + 1] = { '\0' };
    int   ret = ZCC_FILE_ERROR;
    char *ptr = name;
    char *cp;

#if (ZCCDEBUG)
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
                ret = zccreadfile(name, curfile);
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
                while (ret == ZCC_FILE_ERROR) {
                    ret = zccreadfile(name, curfile);
                    name[0] = '\0';
                    cp = strtok(NULL, ":");
                    if (cp) {
                        strcat(name, cp);
                        strcat(name, ptr);
                    } else {

                        return ZCC_FILE_ERROR;
                    }
                }
            } else {
                fprintf(stderr, "invalid #include directive %s\n",
                        name);
                
                exit(1);
            }
        }
    }
    if (ret != ZCC_FILE_ERROR) {
        *retstr = str;
    }

    return ret;
}

static void
zppqueuetoken(struct zpptoken *token, int curfile)
{
    struct zpptokenq *queue = &zccfiletokens[curfile];
    struct zpptoken  *head = queue->head;
    struct zpptoken  *tail = queue->tail;

#if (ZCCPRINT) && 0
    fprintf(stderr, "QUEUE\n");
    fprintf(stderr, "-----\n");
    zppprinttoken(token);
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
zccreadfile(char *name, int curfile)
{
    long              buflen = NLINEBUF;
    FILE             *fp = fopen(name, "r");
    long              eof = 0;
    long              nl = 0;
    struct zpptoken  *tok;
    char             *str = linebuf;
    char             *lim = NULL;
    long              loop = 1;
    int               ch;
    int               tmp;
    long              comm = 0;
    long              done = 1;
    long              len = 0;
#if (ZCCDB)
    unsigned long     line = 0;
#endif

    if (!fp) {

        return ZCC_FILE_ERROR;
    }
    while (loop) {
        if (done) {
            if (nl) {
                tok = malloc(sizeof(struct zpptoken));
                tok->type = ZPP_NEWLINE_TOKEN;
                tok->parm = ZCC_NONE;
                tok->str = NULL;
                tok->data = 0;
                zppqueuetoken(tok, curfile);
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
#if (ZCCDB)
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
        } else {
            tmp = zccgetinclude(str, &str, curfile);
            if (tmp != ZCC_FILE_ERROR) {
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
#if (ZCCDB)
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
                tok = zccgettoken(str, &str, curfile);
                if (tok) {
#if (ZCCDB)
                    tok->fname = strdup(name);
                    tok->line = line;
#endif
                    zppqueuetoken(tok, curfile);
                }
                if (str >= lim) {
                        done = 1;
                }
            } else {
                done = 1;
            }
        }
    }
    fclose(fp);
    
    return curfile;
}

struct zppinput *
zpplex(int argc,
       char *argv[])
{
    struct zppinput *input = NULL;
    int              arg;
    long             l;
    
    arg = zccinit(argc, argv);
    if (!arg) {
        
        exit(1);
    }
    for (l = arg; l < argc ; l++) {
        zcccurfile = zccreadfile(argv[l], zcccurfile);
        if (zcccurfile == ZCC_FILE_ERROR) {
            
            exit(1);
        }
    }
    input = malloc(sizeof(struct zppinput));
    if (input) {
        input->nq = zcccurfile + 1;
        input->qptr = &zccfiletokens;
    }

    return input;
}

