#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <ctype.h>

#include <corewar/cw.h>

extern struct cwinstr *cwoptab;

static void       *rcparsetab[128];
static const char *rcoptab[CWNOP]
= {
    "DAT",
    "MOV",
    "ADD",
    "SUB",
    "JMP",
    "JMZ",
    "JMN",
    "CMP",
    "SLT",
    "DJN",
    "SPL",
};
long               rcnargtab[CWNOP]
= {
    0,
    2,
    2,
    2,
    1,
    2,
    2,
    2,
    2,
    2,
    1
};

void
rcaddop(char *name, long id)
{
    void *ptr1;
    void *ptr2;
    long  ndx = toupper(*name);

    name++;
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
        ndx = toupper(*name);
        name++;
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
            ndx = toupper(*name);
            name++;
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
                *((long *)ptr1) = id;
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
    rcaddop("DAT", CWOPDAT);
    rcaddop("MOV", CWOPMOV);
    rcaddop("ADD", CWOPADD);
    rcaddop("SUB", CWOPSUB);
    rcaddop("JMP", CWOPJMP);
    rcaddop("JMZ", CWOPJMZ);
    rcaddop("JMN", CWOPJMN);
    rcaddop("CMP", CWOPCMP);
    rcaddop("SLT", CWOPSLT);
    rcaddop("DJN", CWOPDJN);
    rcaddop("SPL", CWOPSPL);
}

void
rcdisasm(struct cwinstr *op, FILE *fp)
{
    char ch;

    if (op) {
        fprintf(fp, "\t%s\t", rcoptab[op->op]);
        ch = '\0';
        if (op->aflg & CWIMMBIT) {
            ch = '#';
        } else if (op->aflg & CWINDIRBIT) {
            ch = '@';
        } else if (op->aflg & CWPREDECBIT) {
            ch = '<';
        }
        if (ch) {
            fprintf(fp, "%c", ch);
        }
        fprintf(fp, "%d", op->a);
        if (rcnargtab[op->op] == 2) {
            ch = '\0';
            if (op->bflg & CWIMMBIT) {
                ch = '#';
            } else if (op->bflg & CWINDIRBIT) {
                ch = '@';
            } else if (op->aflg & CWPREDECBIT) {
                ch = '<';
            }
            if (ch) {
                fprintf(fp, "\t%c", ch);
            } else {
                fprintf(fp, "\t");
            }
            fprintf(stderr, "%d\n", op->b);
        } else {
            fprintf(stderr, "\n");
        }
    }

    return;
}

void
rcshowmem(void)
{
    struct cwinstr *op;
    long            l;

    for (l = 0 ; l < CWNCORE ; l++) {
        op = &cwoptab[l];
        if (*(uint64_t *)op) {
            fprintf(stderr, "%ld\t", l);
            rcdisasm(op, stderr);
        }
    }
}

struct cwinstr *
rcgetop(char *str)
{
    char           *cp = str;
    struct cwinstr *instr = NULL;
    long            op;
    long            sign;
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
        op = CWNONE;
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
        instr->aflg = 0;
        instr->bflg = 0;
        instr->a = 0;
        instr->b = 0;
        if (*cp) {
            while (isspace(*cp)) {
                cp++;
            }
            if (*cp) {
                if (*cp == '#') {
                    instr->aflg |= CWIMMBIT;
                    cp++;
                } else if (*cp == '@') {
                    instr->aflg |= CWINDIRBIT;
                    cp++;
                } else if (*cp == '<') {
                    instr->aflg |= CWPREDECBIT;
                    cp++;
                }
                val = CWNONE;
                sign = 0;
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
                } else {
                    fprintf(stderr, "invalid A-field: %s\n", str);
                    
                    exit(1);
                }
                if (sign) {
                    val = -val;
                }
                instr->a = val;
                if (narg == 2) {
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
                    } else if (*cp == '<') {
                        instr->bflg |= CWPREDECBIT;
                        cp++;
                    }
                    val = CWNONE;
                    sign = 0;
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
                    } else {
                        fprintf(stderr, "invalid B-field: %s\n", str);
                        
                        exit(1);
                    }
                    if (sign) {
                        val = -val;
                    }
                    instr->b = val;
                }
            }
        }
    }
    
    return instr;
}

char *
rcgetline(FILE *fp)
{
    char *buf = NULL;
    long  n = 32;
    long  ndx = 0;
    int   ch;

    ch = fgetc(fp);
    if (ch != EOF) {
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
            buf[ndx] = '\n';
        }
    }

    return buf;
}

long
rcxlate(FILE *fp, long pid, long base, long *baseret, long *limret)
{
    char           *linebuf;
    char           *cp;
    struct cwinstr *op;
    struct cwinstr *instr;
    long            ip = base;
    long            ret = -1;
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
                op = rcgetop(cp);
                if (op) {
                    n++;
                    instr = &cwoptab[ip];
                    if (*((uint64_t *)instr)) {
                        fprintf(stderr, "programs overlap\n");
                        
                        exit(1);
                    }
                    cwoptab[ip] = *op;
                    if (ret < 0 && op->op != CWOPDAT) {
                        ret = ip;
                    }
                    ip++;
                    ip &= CWNCORE - 1;
                } else {
                    fprintf(stderr, "invalid instruction: %s\n", linebuf);

                    exit(1);
                }
            } else if (wrap) {

                continue;
            } else {

                break;
            }
        } else {

            break;
        }
    }
    *limret = ip;

    return ret;
}

