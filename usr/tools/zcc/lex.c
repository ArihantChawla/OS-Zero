#include <ctype.h>
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

static int zccreadfile(char *name, int curfile);
static void zccqueuetoken(struct zcctoken *token, int curfile);
#if (ZCCPRINT)
void printtoken(struct zcctoken *token);
void printqueue(struct zcctokenq *queue);
#endif

#define zccisoper(cp)    (opertab[(int)(*(cp))])
#define zccistypedef(cp) (!strncmp(cp, "typedef", 7))
#define zccisstruct(cp)  (!strncmp(cp, "struct", 6))
#define zccisunion(cp)   (!strncmp(cp, "union", 5))
#define zccisatr(cp)     (!strncmp(cp, "__attribute__", 13))
#define zccispreproc(cp) (*str == '#')
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
    ((!strncmp(cp, "if", 2))                                            \
     ? ZCC_IF_DIR                                                       \
     : (!strncmp(cp, "elif", 4)                                         \
        ? ZCC_ELIF_DIR                                                  \
        : (!strncmp(cp, "else", 4)                                      \
           ? ZCC_ELSE_DIR                                               \
           : (!strncmp(cp, "endif", 5)                                  \
              ? ZCC_ENDIF_DIR                                           \
              : (!strncmp(cp, "ifdef", 5)                               \
                 ? ZCC_IFDEF_DIR                                        \
                 : (!strncmp(cp, "ifndef", 6)                           \
                    ? ZCC_IFNDEF_DIR                                    \
                    : (!strncmp(cp, "define", 6)                        \
                       ? ZCC_DEFINE_DIR                                 \
                       : ZCC_NONE)))))))
#define zccatrid(cp)                                                    \
    ((!strncmp(cp, "packed", 6))                                        \
     ? ZCC_ATR_PACKED                                                   \
     : (!strncmp(cp, "aligned", 7)                                      \
        ? ZCC_ATR_ALIGNED                                               \
        : (!strncmp(cp, "__noreturn__", 8)                              \
           ? ZCC_ATR_NORETURN\
           : (!strncmp(cp, "__format__", 10)                            \
              ? ZCC_ATR_FORMAT                                          \
              : ZCC_NONE))))
#define zccisagr(t)      ((t) == ZCC_STRUCT || (t) == ZCC_UNION)

#define zccvalsz(t)      (typesztab[(t)])
#define zcctypesz(tok)   (tok->datasz)
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

static char              linebuf[NLINEBUF] ALIGNED(CLSIZE);
static uint8_t           opertab[256];
static uint8_t           toktab[256];
#if (ZCCPRINT)
static char             *toktypetab[256] =
{
    NULL,
    "ZCC_TYPE_TOKEN",
    "ZCC_TYPEDEF_TOKEN",
    "ZCC_VAR_TOKEN",
    "ZCC_CHAR_TOKEN",
    "ZCC_SHORT_TOKEN",
    "ZCC_INT_TOKEN",
    "ZCC_LONG_TOKEN",
    "ZCC_LONG_LONG_TOKEN",
#if (ZCC_C99_TYPES)
    "ZCC_INT8_TOKEN",
    "ZCC_INT16_TOKEN",
    "ZCC_INT32_TOKEN",
    "ZCC_INT64_TOKEN",
#endif
    "ZCC_STRUCT_TOKEN",
    "ZCC_UNION_TOKEN",
    "ZCC_OPER_TOKEN",
    "ZCC_DOT_TOKEN",
    "ZCC_INDIR_TOKEN",
    "ZCC_ASTERISK_TOKEN",
    "ZCC_COMMA_TOKEN",
    "ZCC_SEMICOLON_TOKEN",
    "ZCC_COLON_TOKEN",
    "ZCC_EXCLAMATION_TOKEN",
    "ZCC_LEFT_PAREN_TOKEN",
    "ZCC_RIGHT_PAREN_TOKEN",
    "ZCC_INDEX_TOKEN",
    "ZCC_END_INDEX_TOKEN",
    "ZCC_BLOCK_TOKEN",
    "ZCC_END_BLOCK_TOKEN",
    "ZCC_QUOTE_TOKEN",
    "ZCC_DOUBLE_QUOTE_TOKEN",
    "ZCC_BACKSLASH_TOKEN",
    "ZCC_VALUE_TOKEN",
    "ZCC_QUAL_TOKEN",
    "ZCC_ATR_TOKEN",
    "ZCC_FUNC_TOKEN",
    "ZCC_LABEL_TOKEN",
    "ZCC_ADR_TOKEN",
    "ZCC_MACRO_TOKEN",
    "ZCC_PREPROC_TOKEN",
    "ZCC_CONCAT_TOKEN",
    "ZCC_STRINGIFY_TOKEN",
    "ZCC_LATIN1_TOKEN",
    "ZCC_UTF8_TOKEN",
    "ZCC_UCS16_TOKEN",
    "ZCC_UCS32_TOKEN"
};
static char             *tokparmtab[256] =
{
    "NONE",
    "ZCC_EXTERN_QUAL",
    "ZCC_STATIC_QUAL",
    "ZCC_CONST_QUAL",
    "ZCC_VOLATILE_QUAL",
    "ZCC_IF_DIR",
    "ZCC_ELIF_DIR",
    "ZCC_ELSE_DIR",
    "ZCC_ENDIF_DIR",
    "ZCC_IFDEF_DIR",
    "ZCC_IFNDEF_DIR",
    "ZCC_DEFINE_DIR"
};
#endif
static long               typesztab[32] = {
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
static long               typesigntab[32] = {
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
#if (ZCCPRINT)
static char              *typetab[32] =
{
    "NONE",
    "ZCC_CHAR",
    "ZCC_UCHAR",
    "ZCC_SHORT",
    "ZCC_USHORT",
    "ZCC_INT",
    "ZCC_UINT",
    "ZCC_LONG",
    "ZCC_ULONG",
    "ZCC_LONGLONG",
    "ZCC_ULONGLONG",
    "ZCC_FLOAT",
    "ZCC_DOUBLE",
    "ZCC_LDOUBLE"
};
#endif
static long               parmlentab[16] = {
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
    6                   // ZCC_DEFINE_DIR
};
static long               atrlentab[16] = {
    0,                  // ZCC_NONE
    6,                  // ZCC_ATR_PACKED
    7,                  // ZCC_ATR_ALIGNED
    8                   // ZCC_ATR_NORETURN
};
static struct zcctokenq *zccfiletokens;
static int               zcccurfile;
static int               zccnfiles;
static long              zccoptflags;
#if (ZCCTOKENCNT)
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
    toktab[';'] = ZCC_SEMICOLON_TOKEN;
    toktab['.'] = ZCC_DOT_TOKEN;
    toktab[','] = ZCC_COMMA_TOKEN;
    toktab[';'] = ZCC_SEMICOLON_TOKEN;
    toktab['{'] = ZCC_BLOCK_TOKEN;
    toktab['}'] = ZCC_END_BLOCK_TOKEN;
    toktab['?'] = ZCC_EXCLAMATION_TOKEN;
    toktab[':'] = ZCC_COLON_TOKEN;
    toktab['('] = ZCC_LEFT_PAREN_TOKEN;
    toktab[')'] = ZCC_RIGHT_PAREN_TOKEN;
    toktab['['] = ZCC_INDEX_TOKEN;
    toktab[']'] = ZCC_END_INDEX_TOKEN;
    toktab['\''] = ZCC_QUOTE_TOKEN;
    toktab['"'] = ZCC_DOUBLE_QUOTE_TOKEN;
    toktab['\\'] = ZCC_BACKSLASH_TOKEN;
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
    zccfiletokens = malloc(NFILE * sizeof(struct zcctokenq));
    zccnfiles = NFILE;

    return l;
}

static int
zccgettype(char *str, char **retstr)
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
    }
    if (type != ZCC_NONE) {
        *retstr = str;
    }

    return type;
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
    if (str[0] == '0' && (str[1] == 'x' || str[1] == 'X')) {
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
    } else if (str[0] == '0' && (str[1] == 'b' || str[1] == 'B')) {
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
            if (*str == 'L' || *str == 'l') {
                str++;
                if (*str == 'L' || *str == 'l') {
                    type = ZCC_LONGLONG;
                } else {
                    type = ZCC_LONG;
                }
            }
        } else {
            if (*str == 'U' || *str == 'u') {
                str++;
                if (*str == 'L' || *str == 'l') {
                    str++;
                    if (*str == 'L' || *str == 'l') {
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
        newval->sz = zccvalsz(type);
        if (neg) {
            zccsetival(newval, type, &val);
        } else {
            zccsetival(newval, type, &uval);
        }
        while (*str == ',' || *str == '\'') {
            str++;
        }
        *retstr = str;
    }

    return newval;
}

static struct zcctoken *
zccgettoken(char *str, char **retstr, int curfile)
{
    long             len = 0;
    long             ndx;
    long             parm;
    long             atr;
    long             type;
    char            *ptr;
    struct zcctoken *token = malloc(sizeof(struct zcctoken));
    struct zccval   *val;

#if (ZCCDEBUG)
    fprintf(stderr, "gettoken: %s\n", str);
#endif
    token->type = ZCC_NONE;
    token->parm = ZCC_NONE;
    token->str = NULL;
    token->adr = NULL;
    if (*str == '-' && str[1] == '>') {
        token->type = ZCC_INDIR_TOKEN;
        str += 2;
    } else if (*str == '*') {
        token->type = ZCC_ASTERISK_TOKEN;
        str++;
    } else if (zccisoper(str)) {
        token->type = ZCC_OPER_TOKEN;
        len = 8;
        ndx ^= ndx;
        token->str = malloc(len);
        ptr = token->str;
        while (zccisoper(str)) {
            if (ndx == len) {
                len <<= 1;
                token->str = realloc(token->str, len);
                ptr = &token->str[ndx];
            }
            *ptr++ = *str++;
            ndx++;
        }
        *ptr = '\0';
    } else if ((type = toktab[(int)(*str)])) {
        token->type = type;
        str++;
    } else if (*str == '#') {
        str++;
        while (isspace(*str)) {
            str++;
        }
        parm = zccpreprocid(str);
        if (parm != ZCC_NONE) {
            len = parmlentab[parm];
            str += len;
        }
        if (parm == ZCC_DEFINE_DIR) {
            if (*str == '(') {
                while (*str == '(') {
                    str++;
                }
            }
            token->type = ZCC_MACRO_TOKEN;
            token->parm = parm;
        } else if (parm != ZCC_NONE) {
            token->type = ZCC_PREPROC_TOKEN;
            token->parm = parm;
            str += len;
        } else if (*str == '#') {
            str++;
            token->type = ZCC_CONCAT_TOKEN;
        } else {
            str++;
            token->type = ZCC_STRINGIFY_TOKEN;
        }
    } else if (zccistypedef(str)) {
        token->type = ZCC_TYPEDEF_TOKEN;
        str += 7;
    } else if (zccisatr(str)) {
        token->type = ZCC_ATR_TOKEN;
        str += 13;
        while (isspace(*str)) {
            str++;
        }
        while (*str == '(') {
            str++;
        }
        if ((atr = zccatrid(str))) {
            len = atrlentab[atr];
        } else {
            fprintf(stderr, "invalid attribute %s\n", str);
        }
        str += len;
    } else if ((parm = zccqualid(str))) {
        len = parmlentab[parm];
        token->type = ZCC_QUAL_TOKEN;
        token->parm = parm;
        str += len;
    } else if (zccisstruct(str)) {
        token->type = ZCC_STRUCT_TOKEN;
        str += 6;
    } else if (zccisunion(str)) {
        token->type = ZCC_UNION_TOKEN;
        str += 5;
    } else if (isalpha(*str) || *str == '_') {
        ptr = str;
        while (isalnum(*str) || *str == '_') {
            str++;
        }
        if (*str == ':') {
            *str = '\0';
            token->type = ZCC_LABEL_TOKEN;
            token->str = strndup(ptr, str - ptr);
            token->adr = ZCC_NO_ADR;
            str++;
        } else {
            while (isspace(*str)) {
                str++;
            }
            token->str = strndup(ptr, str - ptr);
            if (*str == '(') {
                token->type = ZCC_FUNC_TOKEN;
            } else if ((type = zccgettype(ptr, &str))) {
                token->type = ZCC_TYPE_TOKEN;
                token->parm = type;
                token->str = strndup(ptr, str - ptr);
            } else {
                token->type = ZCC_VAR_TOKEN;
                token->adr = ZCC_NO_ADR;
            }
        }
    } else if (isxdigit(*str)) {
        ptr = str;
        val = zccgetval(str, &str);
        if (val) {
            token->type = ZCC_VALUE_TOKEN;
            token->str = strndup(ptr, str - ptr);
            token->adr = val;
        }
    } else {
        free(token);
        token = NULL;
    }
    if (token) {
#if (ZCCTOKENCNT)
        ntoken++;
#endif
        while (isspace(*str)) {
            str++;
        }
        *retstr = str;
    }

    return token;
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
    if (curfile == zccnfiles) {
        zccnfiles <<= 1;
        zccfiletokens = realloc(zccfiletokens,
                                zccnfiles * sizeof(struct zcctokenq));
        if (!zccfiletokens) {
            fprintf(stderr, "cannot include token table\n");

            return ZCC_FILE_ERROR;
        }
    }
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
zccqueuetoken(struct zcctoken *token, int curfile)
{
    struct zcctokenq *queue = &zccfiletokens[curfile];
    struct zcctoken  *head = queue->head;
    struct zcctoken  *tail = queue->tail;

#if (ZCCPRINT) && 0
    fprintf(stderr, "QUEUE\n");
    fprintf(stderr, "-----\n");
    printtoken(token);
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
    struct zcctoken  *token;
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
                    *str++ = ch;
                    len++;
                    if (len == buflen) {
                        fprintf(stderr, "overlong line\n");
                        
                        exit(1);
                    }
                    ch = fgetc(fp);
                    eof = (ch == EOF);
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
            } else {
                if (*str) {
                    token = zccgettoken(str, &str, curfile);
                    if (token) {
#if (ZCCDB)
                        token->fname = strdup(name);
                        token->line = line;
#endif
                        zccqueuetoken(token, curfile);
                    }
                    if (str >= lim) {
                        done = 1;
                    }
                } else {
                    done = 1;
                }
            }
        }
    }
    fclose(fp);
    
    return curfile;
}

#if (ZCCPRINT)

void
printval(struct zccval *val)
{
    switch (val->type) {
        case ZCC_CHAR:
            fprintf(stderr, "CHAR: %x\n", val->ival.c);

            break;
        case ZCC_UCHAR:
            fprintf(stderr, "UCHAR: %x\n", val->ival.uc);

            break;
        case ZCC_SHORT:
            fprintf(stderr, "SHORT: %x\n", val->ival.s);

            break;
        case ZCC_USHORT:
            fprintf(stderr, "USHORT: %x\n", val->ival.us);

            break;
        case ZCC_INT:
            fprintf(stderr, "INT: %x\n", val->ival.i);

            break;
        case ZCC_UINT:
            fprintf(stderr, "UINT: %x\n", val->ival.ui);

            break;
        case ZCC_LONG:
            fprintf(stderr, "LONG: %lx\n", val->ival.l);

            break;
        case ZCC_ULONG:
            fprintf(stderr, "ULONG: %lx\n", val->ival.ul);

            break;
        case ZCC_LONGLONG:
            fprintf(stderr, "LONGLONG: %llx\n", val->ival.ll);


        case ZCC_ULONGLONG:
            fprintf(stderr, "ULONGLONG: %llx\n", val->ival.ull);

            break;

        default:

            break;
    }
    fprintf(stderr, "SZ: %ld\n", val->sz);

    return;
}

void
printtoken(struct zcctoken *token)
{
    fprintf(stderr, "TYPE %s\n", toktypetab[token->type]);
    if (token->type == ZCC_TYPE_TOKEN) {
        fprintf(stderr, "PARM: %s\n", typetab[token->parm]);
    } else {
        fprintf(stderr, "PARM: %s\n", tokparmtab[token->parm]);
    }
    fprintf(stderr, "STR: %s\n", token->str);
    if (token->type == ZCC_VALUE_TOKEN) {
        fprintf(stderr, "VALUE\n");
        fprintf(stderr, "-----\n");
        printval(token->adr);
    } else {
        fprintf(stderr, "ADR: %p\n", token->adr);
    }
}

void
printqueue(struct zcctokenq *queue)
{
    struct zcctoken *token = queue->head;

    while (token) {
        printtoken(token);
        token = token->next;
    }
}

#endif /* ZCCPRINT */

struct zccinput *
zcclex(int argc,
       char *argv[])
{
    struct zccinput *input = NULL;
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
    input = malloc(sizeof(struct zccinput));
    if (input) {
        input->nq = zcccurfile + 1;
        input->qptr = &zccfiletokens;
    }

    return input;
}

