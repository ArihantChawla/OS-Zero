#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#include <corewar/cw.h>

extern struct cwinstr **cwoptab;

static void       *rcparsetab[128];
static const char *rcoptab[CWNOP]
= {
    "DAT",
    "MOV",
    "ADD",
    "SUB",
    "JMP",
    "JMZ",
    "DJZ",
    "CMP"
};
static long        rcnargtab[CWNOP]
= {
    1,
    2,
    2,
    2,
    1,
    2,
    2,
    2
};

void
rcaddop(char *name, long id)
{
    void *ptr1;
    void *ptr2;
    long  ndx = *name++;

    if (ndx) {
        ptr1 = rcparsetab[ndx];
        if (!ptr1) {
            ptr1 = calloc(128, sizeof(void *));
            rcparsetab[ndx] = ptr1;
        }
        if (!ptr1) {
            fprintf(stderr, "failed to allocate operation\n");

            exit(1);
        }
        ndx = *name++;
        if (ndx) {
            ptr2 = ((void **)ptr1)[ndx];
            if (!ptr2) {
                ptr2 = calloc(128, sizeof(void *));
                ((void **)ptr1)[ndx] = ptr2;
            }
            if (!ptr2) {
                fprintf(stderr, "failed to allocate operation\n");
                
                exit(1);
            }
            ndx = *name++;
            if (ndx) {
                ptr1 = ((long **)ptr2)[ndx];
                if (!ptr1) {
                    ptr1 = calloc(128, sizeof(long));
                    ((long **)ptr2)[ndx] = ptr1;
                }
                if (!ptr1) {
                    fprintf(stderr, "failed to allocate operation\n");
                    
                    exit(1);
                }
                ndx = *name++;
                if (ndx) {
                    ((long *)ptr1)[ndx] = id;
                }
            }
        }
    }

    return;
}

long
rcfindop(char *str, long *narg)
{
    long  op = CWNONE;
    char *cp = str;
    void *ptr;

    ptr = rcparsetab[toupper(*cp)];
    cp++;
    if ((ptr) && isalpha(*cp)) {
        ptr = ((void **)ptr)[toupper(*cp)];
        cp++;
        if ((ptr) && isalpha(*cp)) {
            ptr = ((void **)ptr)[toupper(*cp)];
            cp++;
            if (isspace(*cp)) {
                op = *((long *)ptr);
            }
        }
    }
    if (op != CWNONE) {
        *narg = rcnargtab[op];
    }

    return op;
}

void
rcinitop(void)
{
    rcaddop("DAT", 0);
    rcaddop("MOV", 1);
    rcaddop("ADD", 2);
    rcaddop("SUB", 3);
    rcaddop("JMP", 4);
    rcaddop("JMZ", 5);
    rcaddop("DJZ", 6);
    rcaddop("CMP", 7);
}

void
rcdisasm(struct cwinstr *op, FILE *fp)
{
    char ch;

    if (op) {
        fprintf(fp, "\t%s\t", rcoptab[op->op]);
    }
    ch = '\0';
    if (op->aflg & CWIMMBIT) {
        ch = '#';
    } else if (op->aflg & CWINDIRBIT) {
        ch = '@';
    }
    if (ch) {
        fprintf(fp, "%c", ch);
    }
    fprintf(stderr, "%d\t", op->a);
    ch = '\0';
    if (op->bflg & CWIMMBIT) {
        ch = '#';
    } else if (op->bflg & CWINDIRBIT) {
        ch = '@';
    }
    if (ch) {
        fprintf(fp, "%c", ch);
    }
    fprintf(stderr, "%d\n", op->b);

    return;
}

struct cwinstr *
rcgetop(char *str)
{
    char           *cp = str;
    struct cwinstr *instr = NULL;
    long            op = CWNONE;
    long            sign = 0;
    long            val;
    long            narg;

    if (cp) {
        instr = calloc(1, sizeof(struct cwinstr));
        if (!instr) {
            fprintf(stderr, "failed to allocate instruction\n");

            exit(1);
        }
        while (isspace(*cp)) {
            cp++;
        }
        if (isalpha(*cp)) {
            op = rcfindop(cp, &narg);
        }
        if (op != CWNONE) {
            instr->op = op;
            while (isalpha(*cp)) {
                cp++;
            }
        } else {
            fprintf(stderr, "invalid mnemonic: %s\n", str);

            exit(1);
        }
        instr->aflg = CWNONE;
        instr->bflg = CWNONE;
        instr->a = CWNONE;
        instr->b = CWNONE;
        if (*cp) {
            while (isspace(*cp)) {
                cp++;
            }
            if (*cp) {
                if (narg == 2) {
                    if (*cp == '#') {
                        instr->aflg |= CWIMMBIT;
                        cp++;
                    } else if (*cp == '@') {
                        instr->aflg |= CWINDIRBIT;
                        cp++;
                    }
                    val = CWNONE;
                    if (*cp == '-') {
                        sign = 1;
                        cp++;
                    }
                    if (isdigit(*cp)) {
                        val = 0;
                        while (isdigit(*cp)) {
                            val *= 10;
                            val += *cp - '0';
                            cp++;
                        }
                    }
                    if (sign) {
                        val = -val;
                    }
                    if (val != CWNONE && val <= CWNCORE - 1) {
                        instr->a = val;
                    } else {
                        fprintf(stderr, "invalid A-field: %s\n", str);
                        
                        exit(1);
                    }
                }
                while (isspace(*cp)) {
                    cp++;
                }
                if (*cp == ',') {
                    cp++;
                    while (isspace(*cp)) {
                        cp++;
                    }
                }
                if (*cp == '#') {
                    instr->bflg |= CWIMMBIT;
                    cp++;
                } else if (*cp == '@') {
                    instr->bflg |= CWINDIRBIT;
                    cp++;
                }
                val = CWNONE;
                if (*cp == '-') {
                    sign = 1;
                    cp++;
                }
                if (isdigit(*cp)) {
                    val = 0;
                    while (isdigit(*cp)) {
                        val *= 10;
                        val += *cp - '0';
                        cp++;
                    }
                }
                if (sign) {
                    val = -val;
                }
                if (val != CWNONE && val <= CWNCORE - 1) {
                    instr->b = val;
                } else {
                    fprintf(stderr, "invalid B-field: %s\n", str);
                    
                    exit(1);
                }
            }
        }
    }
    
    return instr;
}

char *
rcgetline(FILE *fp)
{
    char *buf;
    long  n = 32;
    long  ndx = 0;
    int   ch = fgetc(fp);

    buf = malloc(n * sizeof(char));
    if (buf) {
        while (ch != EOF) {
            if (ndx == n) {
                n <<= 1;
                buf = realloc(buf, n * sizeof(char));
            }
            if (ch == '\n') {
                
                break;
            }
            buf[ndx] = ch;
            ndx++;
            ch = fgetc(fp);
        }
        buf[ndx] = '\0';
    }

    return buf;
}

long
rcxlate(FILE *fp, long base, long *baseret, long *limret)
{
    char           *linebuf;
    char           *cp;
    struct cwinstr *instr;
    long            ip = base;
    long            n = 0;
    long            wrap = 0;

    *baseret = ip;
    while (1) {
        linebuf = rcgetline(fp);
        if (linebuf) {
            cp = linebuf;
            while (isspace(*cp)) {
                if (*cp == '\n') {
                    wrap = 1;
                }
                cp++;
            }
            if (isalpha(*cp)) {
                instr = rcgetop(cp);
                if (instr) {
                    n++;
                    if (cwoptab[ip]) {
                        fprintf(stderr, "programs overlap\n");
                        
                        exit(1);
                    }
                    cwoptab[ip] = instr;
                    ip++;
                    ip &= CWNCORE - 1;
                }
            } else if (wrap) {

                continue;
            } else {

                break;
            }
        } else {
            *limret = ip;

            return n;
        }
    }
    /* NOTREACHED */
    *limret = CWNONE;

    return n;
}

