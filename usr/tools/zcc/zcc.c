#include <ctype.h>
#include <stdio.h>
#include <stdint.h>
#include <zcc/zcc.h>

#define NVALHASH 1024
#define NTOKENQ  1024

#define zccistypedef(cp) (!strncmp(cp, "typedef", 7))
#define zccisstruct(cp)  (!strncmp(cp, "struct", 6))
#define zccisunion(cp)   (!strncmp(cp, "union", 5))
#define zccisatr(cp)     (!strncmp(cp, "__attribute__", 13))
#define zccqualid(cp)                                                   \
    ((!strncmp(cp, "extern", 6))                                        \
     ? ZCC_EXTERN_QUAL                                                  \
     : (!strncmp(cp, "static", 6)                                       \
        ? ZCC_STATIC_QUAL                                               \
        : (!strncmp(cp, "const", 5)                                     \
           ? ZCC_CONST_QUAL                                             \
           : (!strncmp(cp, "volatile", 8)                               \
              ? ZCC_VOLATILE_QUAL                                       \
              : 0))))
#define zccisagr(t)      ((t) == ZCC_STRUCT || (t) == ZCC_UNION)

#define zcctypesz(tok)   (tok->datasz)
#define zccsetival(vp, t, adr)                                          \
    (typesigntab[(t) & 0x1f]                                            \
     ? ((vp)->ival.ll = *(long long *)(adr))                            \
    : ((vp)->ival.ull = *(unsigned long long *)(adr))
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

/* for type indices, see struct zccval in zcc.h (ZCC_INT etc.) */
static long             typesztab[32] = {
    0,
    1,
    1,
    2,
    2,
    4,
    4,
    LONGSIZE,
    LONGSIZE,
    LONGLONGSIZE,
    LONGLONGSIZE
};
static long             typesigntab[32] = {
    0,
    1,
    0,
    1,
    0,
    1,
    0,
    1,
    0,
    1,
    0
};
static long              quallentab[8] = {
    0,  // ZCC_NONE
    6,  // ZCC_EXTERN_QUAL
    6,  // ZCC_STATIC_QUAL
    5,  // ZCC_CONST_QUAL
    8   // ZCC_VOLATILE_QUAL
};
static struct zcctokenq *zccfiletokens;
static int               zcccurfile;
static int               zccnfiles;

static void
zccusage(void)
{
    fprintf(stderr, "usage %s <options> [file1] .. [fileN]\n");
    fprintf(stderr, "\t-h OR --help\tprint this help message\n");

    return;
}

static int
zccinit(int argc,
           char *argv[])
{
    int          l;
    char        *str;
    struct stat  statbuf;

    if (argc == 1) {
        fprintf(stderr, "%s: arguments needed\n", argv[0]);

        return 0;
    }
    for (l = 1 ; l < argc ; l++) {
        str = argc[l];
        if (*str == '-') {
            if (!strcmp(str, "-h") || !strcmp(str, "--help")) {
                zccusage();
                
                exit(0);
            } else if (!strcmp(str, "-O")) {
                str = argc[l + 1];
            }
        } else {

            break;
        }
    }
    zccfiletokens = malloc(NTOKENQ * sizeof(struct zcctokenq));
    zccntokenq = NTOKENQ;

    return l;
}

/* TODO: floating-point values */
static struct zccval *
zccgetval(char *str, char **retptr)
{
    long               type = ZCC_NONE;
    unsigned long long uval = 0;
    long               found = 0;
    long long          val;
    long               neg = 0;
    struct zccval      *newval;

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
                    type = ZCC_LONG_LONG;
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
                        type = ZCC_UNSIGNED_LONG_LONG;
                    } else {
                        type = ZCC_UNSIGNED_LONG;
                    }
                } else {
                    type = ZCC_UNSIGNED_INT;
                }
            }
        }
        newval->type = type;
        newval->sz = zccvalsz(type);
        zccsetival(newval, type, (neg) ? &val : &uval);
        while (*str == ',' || *str == '\'') {
            str++;
        }
        *retptr = str;
    }

    return newval;
}

static struct zcctoken *
zccgettoken(char *str, char **retstr)
{
    char            *cp = name;
    long             key = ZCC_NONE;
    long             done = 0;
    long             len;
    char            *ptr;
    struct zcctoken *token = malloc(sizeof(struct zcctoken));

    token->parm = ZCC_NONE;
    token->str = NULL;
    token->adr = NULL;
    if (*str == ';') {
        token->type = ZCC_SEMICOLON_TOKEN;
        str++;
    } else if (*str == '{') {
        token->type = ZCC_BLOCK_TOKEN;
        str++;
    } else if (*str == '}') {
        token->type = ZCC_END_BLOCK_TOKEN;
        str++;
    } else if (*str == '?') {
        token->type = ZCC_EXCLAMATION_TOKEN;
        str++;
    } else if (*str == ':') {
        token->type = ZCC_COLON_TOKEN;
        str++;
    } else if (zccisdefine(*str)) {
        token->type = ZCC_DEFINE_TOKEN;
    } else if (zccistypedef(*str)) {
        token->type = ZCC_TYPEDEF_TOKEN;
        str += 7;
    } else if (zccisatr(*str)) {
        token->type = ZCC_ATR_TOKEN;
        str += 13;
    } else if (parm = zccqualid(str)) {
        len = quallentab[parm];
        token->type = ZCC_QUAL_TOKEN;
        token->parm = parm;
        str += len;
    } else if (zccisstruct(*str)) {
        token->type = ZCC_STRUCT_TOKEN;
        str += 6;
    } else if (zccisunion(*str)) {
        token->type = ZCC_UNION_TOKEN;
        str += 5;
    } else if (isalpha(*str) || *str == '_') {
        ptr = cp;
        while (isalnum(*str) || *str == '_') {
            str++;
        }
        if (*str == ':') {
            token->type = ZCC_LABEL_TOKEN;
            token->str = strndup(ptr, str - ptr);
            token->adr = ZCC_NO_ADR;
            str++;
        } else {
            token->type = ZCC_VAR_TOKEN;
            token->str = strndup(ptr, str - ptr);
            token->adr = ZCC_NO_ADR;
        }
    } else if (isxdigit(*str)) {
        ptr = str;
        val = zccgetval(str, &str);
        if (val) {
            token->type = ZCC_VALUE_TOKEN;
            token->str = strndup(name, str - ptr);
            token->adr = val;
        }
    } else {
        free(token);
        token = NULL;
    }
    if (token) {
        *retstr = str;
    }

    return token;
}

static int
zccgetinclude(char *str, char **retstr, int curfile)
{    
    int ret = 0;

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
        if (*str == '<') {
            str++;
            fname = str;
            while ((*str) && *str != '>') {
                str++;
            }
            if (*str == '>') {
                *str = '\0';
                ret = zccreadfile((char *)fname, curfile);
            } else {
                fprintf(stderr, "invalid #include directive %s\n",
                        str);
                
                exit(1);
            }
        }
    }

    return ret;
}

static int
zccreadfile(char *name, int curfile)
{
    long              buflen = ZCCLINELEN;
    FILE             *fp = fopen((char *)name, "r");
    long              eof = 0;
    struct zccval    *def;
    long              type;
    char             *fname;
    char             *ptr;
    char             *str = linebuf;
    char             *lim = NULL;
    long              loop = 1;
    int               ch;
    long              comm = 0;
    long              done = 1;
    long              len = 0;
    long              type;
#if (ZCCDB)
    unsigned long     line = 0;
#endif
    long              val = 0;

    if (!fp) {
        fprintf(stderr, "cannot open %s\n", name);

        return 0;
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
            curfile = zccgetinclude(str, &str, curfile + 1);
            if (curfile) {
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
                    token = zccgettoken(str, &str);
                    if (token) {
#if (ZCCDB)
                        token->fname = strdup((char *)name);
                        token->line = line;
#endif
                        zccqueuetoken(token, curfile);
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
    }
    fclose(fp);
    
    return curfile;
}

int
main(int argc,
     char *argv[])
{
    int arg;

    
    arg = zccinit(argc, argv);
    if (!arg) {

        exit(1);
    }
    for ( ; arg < argc ; arg++) {
        zcccurfile = zccreadfile(argv[l], zcccurfile);
        if (zcccurfile == ZCC_FILE_ERROR) {

            exit(1);
        }
    }
}

