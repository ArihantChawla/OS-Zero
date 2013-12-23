/* REFERENCE:  http://corewar.co.uk/cwg.txt */
/* REFERENCE: http://seblog.cs.uni-kassel.de/fileadmin/se/courses/SE1/WS0708/redcode-icws-88-2.pdf */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include <zero/param.h>
#include <zero/cdecl.h>
#if (CWRANDMT32)
#include <zero/randmt32.h>
#endif

#include <corewar/cw.h>
#include <corewar/rc.h>
#if (ZEUS)
#include <unistd.h>
#include <corewar/zeus.h>
#endif

typedef long cwinstrfunc(long, long);

extern long         rcnargtab[CWNOP];

const char         *cwopnametab[CWNOP]
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
static long         cwrunqueue[2][CWNPROC];
static long         cwproccnt[2];
static long         cwcurproc[2];
static cwinstrfunc *cwfunctab[CWNOP];
#if (ZEUS)
struct zeusx11     *zeusx11;
#endif
struct cwinstr     *cwoptab;

void
cwdisasm(struct cwinstr *op, FILE *fp)
{
    char ch;

    if (op) {
        fprintf(fp, "\t%s\t", cwopnametab[op->op]);
        if  (rcnargtab[op->op] == 2) {
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
            if (op->aflg & CWSIGNBIT) {
                fprintf(fp, "%d", op->a - CWNCORE);
            } else {
                fprintf(fp, "%d", op->a);
            }
        }
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
        if (op->bflg & CWSIGNBIT) {
            fprintf(fp, "%d\n", op->b - CWNCORE);
        } else {
            fprintf(stderr, "%d\n", op->b);
        }
    }

    return;
}

void
cwgetargs(struct cwinstr *op, long pc, long *argp1, long *argp2)
{
    long arg1 = 0;
    long arg2 = 0;
    long tmp;
    
    if ((op)->aflg & CWIMMBIT) {
        arg1 = (op)->a;
    } else {
        tmp = (pc) + (op)->a;
        tmp %= CWNCORE;
        if ((op)->aflg & (CWINDIRBIT | CWPREDECBIT)) {
            struct cwinstr *ptr;
            
            ptr = &cwoptab[tmp];
            tmp = ptr->b;
            if ((op)->aflg & CWPREDECBIT) {
                tmp--;
                ptr->b = tmp;
            }
            arg1 = tmp;
            arg1 += (pc);
        } else {
            arg1 = tmp;
        }
    }
    arg1 %= CWNCORE;
    if ((op)->bflg & CWIMMBIT) {
        arg2 = (op)->b;
    } else {
        tmp = (pc) + (op)->b;
        tmp %= CWNCORE;
        if ((op)->bflg & (CWINDIRBIT | CWPREDECBIT)) {
            struct cwinstr *ptr;
            
            ptr = &cwoptab[tmp];
            tmp = ptr->b;
            if ((op)->bflg & CWPREDECBIT) {
                tmp--;
                ptr->b = tmp;
            }
            arg2 = tmp;
            arg2 += (pc);
        } else {
            arg2 = tmp;
        }
    }
    arg2 %= CWNCORE;
    *argp1 = arg1;
    *argp2 = arg2;

    return;
}

long
cwdatop(long pid, long pc)
{
#if (ZEUS)
    zeusdrawsim(zeusx11);
#endif
    if (pid) {
        fprintf(stderr, "program #1 won\n");
    } else {
        fprintf(stderr, "program #2 won\n");
    }
#if (ZEUS)
    sleep(5);
#endif
    exit(0);

    /* NOTREACHED */
    return CWNONE;
}

long
cwmovop(long pid, long pc)
{
    struct cwinstr *op = &cwoptab[pc];
    long            arg1;
    long            arg2;
    
    cwgetargs(op, pc, &arg1, &arg2);
    if (op->aflg & CWIMMBIT) {
        if (op->bflg & CWIMMBIT) {
            cwoptab[arg2] = cwoptab[arg1];
        } else {
            cwoptab[arg2].b = arg1;
        }
    } else {
        cwoptab[arg2] = cwoptab[arg1];
    }
    pc++;
    pc %= CWNCORE;
    
    return pc;
}

long
cwaddop(long pid, long pc)
{
    struct cwinstr *op = &cwoptab[pc];
    long            arg1;
    long            arg2;
    long            a;
    long            b;
    
    cwgetargs(op, pc, &arg1, &arg2);
    if (op->aflg & CWIMMBIT) {
        a = arg1;
        if (op->bflg & CWIMMBIT) {
            b = arg2;
        } else {
            b = cwoptab[arg2].b;
        }
        b += a;
        b %= CWNCORE;
        if (op->bflg & CWIMMBIT) {
            op->bflg &= ~CWSIGNBIT;
            op->b = b;
        } else {
            cwoptab[arg2].bflg &= ~CWSIGNBIT;
            cwoptab[arg2].b = b;
        }
    } else if (op->bflg & CWIMMBIT) {
        a = arg1;
        b = arg2;
        b += a;
        b %= CWNCORE;
        op->b = b;
    } else {
        a = cwoptab[arg1].a;
        b = cwoptab[arg1].b;
        a += cwoptab[arg2].a;
        b += cwoptab[arg2].b;
        a %= CWNCORE;
        b %= CWNCORE;
        cwoptab[arg2].a = a;
        cwoptab[arg2].b = b;
    }
    pc++;
    pc %= CWNCORE;
    
    return pc;
}

long
cwsubop(long pid, long pc)
{
    struct cwinstr *op = &cwoptab[pc];
    long            arg1;
    long            arg2;
    long            a;
    long            b;
    
    cwgetargs(op, pc, &arg1, &arg2);
    if (op->aflg & CWIMMBIT) {
        a = arg1;
        b = cwoptab[arg2].b;
        b -= a;
        if (b < 0) {
            b += CWNCORE;
        }
        cwoptab[arg2].b = b;
    } else {
        a = cwoptab[arg1].a;
        b = cwoptab[arg1].b;
        a -= cwoptab[arg2].a;
        b -= cwoptab[arg2].b;
        if (a < 0) {
            a += CWNCORE;
        }
        if (b < 0) {
            b += CWNCORE;
        }
        cwoptab[arg2].a = a;
        cwoptab[arg2].b = b;
    }
    pc++;
    pc %= CWNCORE;
    
    return pc;
}

long
cwjmpop(long pid, long pc)
{
    struct cwinstr *op = &cwoptab[pc];
    long            cnt;
    long            arg1;
    long            arg2;
    
    cwgetargs(op, pc, &arg1, &arg2);
    cnt = cwproccnt[pid];
    if (cnt < CWNPROC) {
        pc = arg2;
        cwrunqueue[pid][cnt - 1] = pc;
    }
    
    return pc;
}

long
cwjmzop(long pid, long pc)
{
    struct cwinstr *op = &cwoptab[pc];
    long            cnt;
    long            arg1;
    long            arg2;
    long            b;
    
    cwgetargs(op, pc, &arg1, &arg2);
    b = cwoptab[arg2].b;
    if (!b) {
        cnt = cwproccnt[pid];
        pc = arg1;
        cwrunqueue[pid][cnt - 1] = pc;
    } else {
        pc++;
        pc %= CWNCORE;
    }
    
    return pc;
}

long
cwjmnop(long pid, long pc)
{
    struct cwinstr *op = &cwoptab[pc];
    long            cnt;
    long            arg1;
    long            arg2;
    long            b;
    
    cwgetargs(op, pc, &arg1, &arg2);
    b = cwoptab[arg2].b;
    if (b) {
        cnt = cwproccnt[pid];
        pc = arg1;
        cwrunqueue[pid][cnt - 1] = pc;
    } else {
        pc++;
        pc %= CWNCORE;
    }
    
    return pc;
}

long
cwcmpop(long pid, long pc)
{
    struct cwinstr *op = &cwoptab[pc];
    long            arg1;
    long            arg2;
    long            a;
    long            b;
    
    cwgetargs(op, pc, &arg1, &arg2);
    if (op->aflg & CWIMMBIT) {
        b = cwoptab[arg2].b;
        if (arg1 == b) {
            pc++;
        }
    } else {
        a = arg1;
        b = arg2;
        if (cwoptab[a].a == cwoptab[b].a && cwoptab[a].b == cwoptab[b].b) {
            pc++;
        }
    }
    pc++;
    pc %= CWNCORE;
    
    return pc;
}

long
cwsltop(long pid, long pc)
{
    struct cwinstr *op = &cwoptab[pc];
    long            arg1;
    long            arg2;
    long            b;
    
    cwgetargs(op, pc, &arg1, &arg2);
    b = cwoptab[arg2].b;
    if (op->aflg & CWIMMBIT) {
        if (arg1 < b) {
            pc++;
        }
    } else if (cwoptab[arg2].b < b) {
        pc++;
    }
    pc++;
    pc %= CWNCORE;
    
    return pc;
}

long
cwdjnop(long pid, long pc)
{
    struct cwinstr *op = &cwoptab[pc];
    long            cnt;
    long            arg1;
    long            arg2;
    long            b;
    
    cwgetargs(op, pc, &arg1, &arg2);
    if (op->bflg & CWIMMBIT) {
        b = cwoptab[arg1].b;
        b--;
        if (b < 0) {
            b += CWNCORE;
        }
        cwoptab[arg1].b = b;
    } else {
        b = cwoptab[arg2].b;
        b--;
        if (b < 0) {
            b += CWNCORE;
        }
        cwoptab[arg2].b = b;
    }
    if (b) {
        cnt = cwproccnt[pid];
        pc = arg1;
        cwrunqueue[pid][cnt - 1] = pc;
    }
    
    return pc;
}

long
cwsplop(long pid, long pc)
{
    struct cwinstr *op = &cwoptab[pc];
    long            cnt;
    long            cur;
    long            arg1;
    long            arg2;
    
    cwgetargs(op, pc, &arg1, &arg2);
    pc++;
    pc %= CWNCORE;
    cnt = cwproccnt[pid];
    cur = cwcurproc[pid];
#if 0
    for (ndx = cur ; ndx < cnt ; ndx++) {
        cwrunqueue[pid][ndx] = cwrunqueue[pid][ndx + 1];
    }
#endif
    cwrunqueue[pid][cur] = pc;
    if (cnt < CWNPROC) {
#if 0
        for (ndx = cur + 1 ; ndx < cnt ; ndx++) {
            cwrunqueue[pid][ndx] = cwrunqueue[pid][ndx + 1];
        }
#endif
        cwrunqueue[pid][cnt] = arg2;
        cnt++;
        cwproccnt[pid] = cnt;
    }
    
    return pc;
}

void
cwinitop(void)
{
    cwfunctab[CWOPDAT] = cwdatop;
    cwfunctab[CWOPMOV] = cwmovop;
    cwfunctab[CWOPADD] = cwaddop;
    cwfunctab[CWOPSUB] = cwsubop;
    cwfunctab[CWOPJMP] = cwjmpop;
    cwfunctab[CWOPJMZ] = cwjmzop;
    cwfunctab[CWOPJMN] = cwjmnop;
    cwfunctab[CWOPCMP] = cwcmpop;
    cwfunctab[CWOPSLT] = cwsltop;
    cwfunctab[CWOPDJN] = cwdjnop;
    cwfunctab[CWOPSPL] = cwsplop;
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
cwexec(long pid)
{
    struct cwinstr *op;
    cwinstrfunc    *func;
    long            cur;
    long            cnt;
    long            pc;
    long            l;
#if (ZEUS)
    static long     ref = 0;
#endif

    cur = cwcurproc[pid];
    pc = cwrunqueue[pid][cur];
    op = &cwoptab[pc];
#if (ZEUS) && 0
    fprintf(stderr, "%ld\t%ld\t", pid, pc);
    cwdisasm(op, stderr);
#endif
    if (!(*((uint64_t *)op))) {
#if (ZEUS)
        zeusdrawsim(zeusx11);
#endif
        if (pid == 0) {
            fprintf(stderr, "program #2 won (%ld)\n", pc);
        } else {
            fprintf(stderr, "program #1 won (%ld)\n", pc);
        }
#if (ZEUS)
        sleep(5);
#endif
        
        exit(0);
    }
#if (ZEUS)
    zeusdrawdb(zeusx11, pc);
#endif
    func = cwfunctab[op->op];
    pc = func(pid, pc);
    cnt = cwproccnt[pid];
    if (pc == CWNONE) {
        if (cnt > 1) {
            for (l = cur ; l < cnt - 1 ; l++) {
                cwrunqueue[pid][l] = cwrunqueue[pid][l + 1];
            }
        } else {
#if (ZEUS)
            zeusdrawsim(zeusx11);
#endif
            if (!pid) {
                fprintf(stderr, "program #2 won\n");
            } else {
                fprintf(stderr, "program #1 won\n");
            }
#if (ZEUS)
            sleep(5);
#endif
            
            exit(0);
        }
        cnt--;
        cwproccnt[pid] = cnt;
    } else if (op->op != CWOPSPL) {
        cwrunqueue[pid][cur] = pc;
        cur++;
    }
    cnt = cwproccnt[pid];
    if (cur == cnt) {
        cur = 0;
    }
    cwcurproc[pid] = cur;
#if (ZEUS)
    ref++;
    if (ref == 32) {
        zeusdrawsim(zeusx11);
        ref = 0;
    }
    if (XEventsQueued(zeusx11->disp, QueuedAfterFlush)) {
        zeusprocev(zeusx11);
    }
#endif

    return;
}

void
cwloop(void)
{
#if (CWRANDMT32)
    long first = randmt32() & 0x01;
#else
    long first = rand() & 0x01;
#endif
    long n;

    if (!first) {
        cwexec(0);
    }
    cwexec(1);
    n = CWNTURN - 1;
    while (n--) {
        cwexec(0);
        cwexec(1);
    }
    fprintf(stderr, "TIE\n");
#if (ZEUS)
    sleep(5);
#endif
    
    exit(0);
}

void
cwinit(void)
{
    time_t seed32 = (((time(NULL) & 0xff) << 24)
                     | ((time(NULL) & 0xff) << 16)
                     | ((time(NULL) & 0xff) << 8)
                     | (time(NULL) & 0xff));

#if (CWRANDMT32)
    srandmt32(seed32);
#else
    srand(time(&seed32));
#endif
    cwinitop();
    rcinitop();
    cwoptab = calloc(CWNCORE, sizeof(struct cwinstr));
    if (!cwoptab) {
        fprintf(stderr, "failed to allocate core\n");
        
        exit(1);
    }

    return;
}

int
main(int argc, char *argv[])
{
    FILE *fp;
    long  base;
    long  lim;
    long  pc1;
    long  pc2;

#if (ZEUS)
    zeusx11 = zeusinitx11();
#endif
    if (argc != 3) {
        fprintf(stderr, "usage: %s prog1.rc prog2.rc\n", argv[0]);
        
        exit(1);
    }
    cwinit();
#if (CWRANDMT32)
    base = randmt32() % CWNCORE;
#else
    base = rand() % CWNCORE;
#endif
    fp = fopen(argv[1], "r");
    if (!fp) {
        fprintf(stderr, "failed to open %s\n", argv[1]);
        
        exit(1);
    }
    pc1 = rcxlate(fp, 0, base, &base, &lim);
    if (pc1 < 0) {
        fprintf(stderr, "failed to translate %s\n", argv[1]);

        exit(1);
    }
    fclose(fp);
#if 0
    if (lim < base) {
#if (CWRANDMT32)
        base = lim + randmt32() % ((base - lim) >> 2);
#else
        base = lim + rand() % ((base - lim) >> 2);
#endif
    } else if (lim - base < CWNCORE - lim) {
#if (CWRANDMT32)
        base = lim + randmt32() % ((CWNCORE - lim) >> 2);
#else
        base = lim + rand() % ((CWNCORE - lim) >> 2);
#endif
    } else {
#if (CWRANDMT32)
        base = randmt32() % (base >> 2);
#else
        base = rand() % (base >> 2);
#endif
    }
#endif
#if (CWRANDMT32)
    base = randmt32() % CWNCORE;
#else
    base = rand() % CWNCORE;
#endif
    fp = fopen(argv[2], "r");
    if (!fp) {
        fprintf(stderr, "failed to open %s\n", argv[2]);

        exit(1);
    }
    pc2 = rcxlate(fp, 1, base, &base, &lim);
    if (pc2 < 0) {
        fprintf(stderr, "failed to translate %s\n", argv[1]);

        exit(1);
    }
    fclose(fp);
    cwcurproc[0] = 0;
    cwcurproc[1] = 0;
    cwproccnt[0] = 1;
    cwproccnt[1] = 1;
    cwrunqueue[0][0] = pc1;
    cwrunqueue[1][0] = pc2;
    cwloop();

    /* NOTREACHED */
    exit(0);
}

