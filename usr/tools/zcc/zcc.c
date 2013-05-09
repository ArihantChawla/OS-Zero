#include <ctype.h>
#include <stdio.h>
#include <stdint.h>
#include <zcc/zcc.h>

#define NVALHASH 1024

#define zccvalsz(t) (typesztab[(t) & 0x1f])
#define zccsetival(vp, t, adr)                                          \
    (typesigntab[(t) & 0x1f]                                            \
    ? ((vp)->ival.ll = *((long long *)adr))                             \
    : ((vp)->ival.ull = *((unsigned long long *adr))))

static struct sym *valhash[NVALHASH];
/* for type indices, see struct zccval in zcc.h (ZCC_INT etc.) */
static long        typesztab[32] = {
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
static long        typesigntab[32] = {
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

static void
zccusage(void)
{
    fprintf(stderr, "usage %s <options> [file1] .. [fileN]\n");
    fprintf(stderr, "\t-h OR --help\tprint this help message\n");

    return;
}

static long
zccprocopt(int argc,
           char *argv[])
{
    long  retval = -1;
    long  l;
    char *str;

    if (argc == 1) {
        fprintf(stderr, "%s: arguments needed\n", argv[0]);

        exit(1);
    }
    for (l = 1 ; l < argc ; l++) {
        str = argc[l];
        if (!strcmp(str, "-h") || !strcmp(str, "--help")) {
            zccusage();

            exit(0);
        }
    }

    return retval;
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

static struct zccmacro *
zccgetmacro(char *str, char **retstr)
{
    struct zccmacro *macro;
    struct zccval   *val;

    if (!strncmp((char *)str, "#define", 7)) {
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
            while ((*str) && isspace(*str)) {
                str++;
            }
            if (*str == '(') {
                fprintf(stderr, "implement macros with arguments\n");

                exit(1);
            } else {
                *str++ = '\0';
                while ((*str) && isspace(*str)) {
                    str++;
                }
                val = zccgetval(str, &str);
                if (val) {
                    macro = malloc(sizeof(struct zccmacro));
                    if (!macro) {
                        fprintf(stderr, "cannot allocate macro\n");
                        
                        exit(1);
                    }
//                macro->type = ZCC_CONST_MACRO;
                    macro->name = strdup((char *)ptr);
                    macro->namelen = strlen(ptr);
#if 0
                    macro->fname = name;
                    macro->fnamelen = strlen(name);
#endif
                    macro->val = val;
                    zccaddmacro(macro);
                } else {
                    fprintf(stderr, "invalid #define directive %s\n", ptr);
                    
                    exit(1);
                }
            }
        }
    }

    return retval;
}

static uintptr_t
zccgetinclude(char *str, char **retstr, uintptr_t adr)
{    
    uintptr_t retval = 0;

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
                zccreadfile((char *)fname, adr);
                retval = zccresolve(adr);
                zccremovesyms();
            } else {
                fprintf(stderr, "invalid #include directive %s\n",
                        str);
                
                exit(1);
            }
        }
    }

    return retval;
}

static void
zccreadfile(char *name, uintptr_t adr)
{
    long             buflen = ZCCLINELEN;
    FILE            *fp = fopen((char *)name, "r");
    long             eof = 0;
    struct zcctoken *token = NULL;
    struct zccval   *def;
    long             type;
    char            *fname;
    char            *ptr;
    char            *str = linebuf;
    char            *lim = NULL;
    long             loop = 1;
    int              ch;
    long             comm = 0;
    long             done = 1;
    long             len = 0;
    long             type;
#if (ZCCDB)
    unsigned long    line = 0;
#endif
    long             val = 0;

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
        } else if (zccgetmacro(str, &str)) {
            done = 1;
        } else if ((zccgetinclude(str, &str, adr))
                   || (str[0] == '/' && str[1] == '/')) {
            /* file included or comment starts */
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
                        zccqueuetoken(token);
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
    fclose(fp);
    
    return;
}

int
main(int argc,
     char *argv[])
{
    struct zcctoken *tokq;

    if (zccprocopt(argc, argv)) {

        exit(1);
    }
    tokq = zcctokenize(argc, argv);
}

