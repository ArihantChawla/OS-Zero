/* REFERENCE:  http://corewar.co.uk/cwg.txt */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include <zero/param.h>
#include <zero/cdecl.h>

#include <corewar/cw.h>
#include <corewar/rc.h>

typedef long cwinstrfunc(long, long);

extern long         rcnargtab[CWNOP];

static long         cwrunqueue[2][CWNPROC];
static long         cwproccnt[2];
static long         cwcurproc[2];
static cwinstrfunc *cwfunctab[CWNOP];
struct cwinstr     *cwoptab;

#if 0
#define cwgetargs(op, ip, arg1, arg2)                                   \
    do {                                                                \
        long _arg1 = 0;                                                 \
        long _arg2 = 0;                                                 \
        long _tmp;                                                      \
                                                                        \
        if ((op)->aflg & CWIMMBIT) {                                    \
            _arg1 = (op)->a;                                            \
        } else {                                                        \
            _tmp = (ip) + (op)->a;                                      \
            _tmp &= CWNCORE - 1;                                        \
            if ((op)->aflg & (CWINDIRBIT | CWPREDECBIT)) {              \
                struct cwinstr *_ptr;                                   \
                                                                        \
                _ptr = &cwoptab[_tmp];                                  \
                _tmp = _ptr->b;                                         \
                if ((op)->aflg & CWPREDECBIT) {                         \
                    _tmp--;                                             \
                    _ptr->b = _tmp;                                     \
                }                                                       \
                _arg1 = _tmp;                                           \
                _arg1 += (ip);                                          \
            } else {                                                    \
                _arg1 = _tmp;                                           \
            }                                                           \
        }                                                               \
        _arg1 &= (CWNCORE - 1);                                         \
        if ((op)->bflg & CWIMMBIT) {                                    \
            _arg2 = (op)->b;                                            \
        } else {                                                        \
            _tmp = (ip) + (op)->b;                                      \
            _tmp &= CWNCORE - 1;                                        \
            if ((op)->bflg & (CWINDIRBIT | CWPREDECBIT)) {              \
                struct cwinstr *_ptr;                                   \
                                                                        \
                _ptr = &cwoptab[_tmp];                                  \
                _tmp = _ptr->b;                                         \
                if ((op)->bflg & CWPREDECBIT) {                         \
                    _tmp--;                                             \
                    _ptr->b = _tmp;                                     \
                }                                                       \
                _arg2 = _tmp;                                           \
                _arg2 += (ip);                                          \
            } else {                                                    \
                _arg2 = _tmp;                                           \
            }                                                           \
        }                                                               \
        _arg2 &= CWNCORE - 1;                                           \
        (arg1) = _arg1;                                                 \
        (arg2) = _arg2;                                                 \
    } while (0)
#endif

void
cwgetargs(struct cwinstr *op, long ip, long *argp1, long *argp2)
{
        long arg1 = 0;
        long arg2 = 0;
        long tmp;

        if ((op)->aflg & CWIMMBIT) {
            arg1 = (op)->a;
        } else {
            tmp = (ip) + (op)->a;
            tmp &= CWNCORE - 1;
            if ((op)->aflg & (CWINDIRBIT | CWPREDECBIT)) {
                struct cwinstr *ptr;

                ptr = &cwoptab[tmp];
                tmp = ptr->b;
                if ((op)->aflg & CWPREDECBIT) {
                    tmp--;
                    ptr->b = tmp;
                }
                arg1 = tmp;
                arg1 += (ip);
            } else {
                arg1 = tmp;
            }
        }
        arg1 &= (CWNCORE - 1);
        if ((op)->bflg & CWIMMBIT) {
            arg2 = (op)->b;
        } else {
            tmp = (ip) + (op)->b;
            tmp &= CWNCORE - 1;
            if ((op)->bflg & (CWINDIRBIT | CWPREDECBIT)) {
                struct cwinstr *ptr;

                ptr = &cwoptab[tmp];
                tmp = ptr->b;
                if ((op)->bflg & CWPREDECBIT) {
                    tmp--;
                    ptr->b = tmp;
                }
                arg2 = tmp;
                arg2 += (ip);
            } else {
                arg2 = tmp;
            }
        }
        arg2 &= CWNCORE - 1;
        *argp1 = arg1;
        *argp2 = arg2;
}

long
cwdatop(long pid, long ip)
{
    if (pid) {
        fprintf(stderr, "program #1 won\n");
    } else {
        fprintf(stderr, "program #2 won\n");
    }
    exit(0);

    /* NOTREACHED */
    return CWNONE;
}

long
cwmovop(long pid, long ip)
{
    struct cwinstr *op = &cwoptab[ip];
    long            ofs;
    long            arg1;
    long            arg2;
    
    cwgetargs(op, ip, &arg1, &arg2);
    if (op->aflg & CWIMMBIT) {
        if (op->bflg & CWIMMBIT) {
            cwoptab[arg2] = cwoptab[arg1];
        } else {
            cwoptab[arg2].b = arg1;
        }
    } else {
        cwoptab[arg2] = cwoptab[arg1];
    }
    ip++;
    ip &= CWNCORE - 1;
    
    return ip;
}

long
cwaddop(long pid, long ip)
{
    struct cwinstr *op = &cwoptab[ip];
    long            arg1;
    long            arg2;
    long            a;
    long            b;
    
    cwgetargs(op, ip, &arg1, &arg2);
    if (op->aflg & CWIMMBIT) {
        a = arg1;
        if (op->bflg & CWIMMBIT) {
            b = arg2;
        } else {
            b = cwoptab[arg2].b;
        }
        b += a;
        b &= CWNCORE - 1;
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
        b &= CWNCORE - 1;
        op->b = b;
    } else {
        a = cwoptab[arg1].a;
        b = cwoptab[arg1].b;
        a += cwoptab[arg2].a;
        b += cwoptab[arg2].b;
        a &= CWNCORE - 1;
        b &= CWNCORE - 1;
        cwoptab[arg2].a = a;
        cwoptab[arg2].b = b;
    }
    ip++;
    ip &= CWNCORE - 1;
    
    return ip;
}

long
cwsubop(long pid, long ip)
{
    struct cwinstr *op = &cwoptab[ip];
    long            arg1;
    long            arg2;
    long            a;
    long            b;
    
    cwgetargs(op, ip, &arg1, &arg2);
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
    ip++;
    ip &= CWNCORE - 1;
    
    return ip;
}

long
cwjmpop(long pid, long ip)
{
    struct cwinstr *op = &cwoptab[ip];
    long            cnt;
    long            arg1;
    long            arg2;
    
    cwgetargs(op, ip, &arg1, &arg2);
    cnt = cwproccnt[pid];
    if (cnt < CWNPROC) {
        ip = arg1;
        cwrunqueue[pid][cnt - 1] = ip;
    }
#if 0
    ip++;
    ip &= CWNCORE - 1;
#endif
    
    return ip;
}

long
cwjmzop(long pid, long ip)
{
    struct cwinstr *op = &cwoptab[ip];
    long            cnt;
    long            arg1;
    long            arg2;
    long            b;
    
    cwgetargs(op, ip, &arg1, &arg2);
    b = cwoptab[arg2].b;
    if (!b) {
        cnt = cwproccnt[pid];
        ip = arg1;
        cwrunqueue[pid][cnt - 1] = ip;
    } else {
        ip++;
        ip &= CWNCORE - 1;
    }
    
    return ip;
}

long
cwjmnop(long pid, long ip)
{
    struct cwinstr *op = &cwoptab[ip];
    long            cnt;
    long            arg1;
    long            arg2;
    long            b;
    
    cwgetargs(op, ip, &arg1, &arg2);
    b = cwoptab[arg2].b;
    if (b) {
        cnt = cwproccnt[pid];
        ip = arg1;
        cwrunqueue[pid][cnt - 1] = ip;
    } else {
        ip++;
        ip &= CWNCORE - 1;
    }
    
    return ip;
}

long
cwcmpop(long pid, long ip)
{
    struct cwinstr *op = &cwoptab[ip];
    long            arg1;
    long            arg2;
    long            a;
    long            b;
    
    cwgetargs(op, ip, &arg1, &arg2);
    if (op->aflg & CWIMMBIT) {
        b = cwoptab[arg2].b;
        if (arg1 == b) {
            ip++;
        }
    } else {
        a = arg1;
        b = arg2;
        if (cwoptab[a].a == cwoptab[b].a && cwoptab[a].b == cwoptab[b].b) {
            ip++;
        }
    }
    ip++;
    ip &= CWNCORE - 1;
    
    return ip;
}

long
cwsltop(long pid, long ip)
{
    struct cwinstr *op = &cwoptab[ip];
    long            arg1;
    long            arg2;
    long            b;
    
    cwgetargs(op, ip, &arg1, &arg2);
    b = cwoptab[arg2].b;
    if (op->aflg & CWIMMBIT) {
        if (arg1 < b) {
            ip++;
        }
    } else if (cwoptab[arg2].b < b) {
        ip++;
    }
    ip++;
    ip &= CWNCORE - 1;
    
    return ip;
}

long
cwdjnop(long pid, long ip)
{
    struct cwinstr *op = &cwoptab[ip];
    long            cnt;
    long            arg1;
    long            arg2;
    long            b;
    
    cwgetargs(op, ip, &arg1, &arg2);
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
        ip = arg1;
        cwrunqueue[pid][cnt - 1] = ip;
    }
    
    return ip;
}

long
cwsplop(long pid, long ip)
{
    struct cwinstr *op = &cwoptab[ip];
    long            cnt;
    long            arg1;
    long            arg2;
    
    cwgetargs(op, ip, &arg1, &arg2);
    ip++;
    ip &= CWNCORE - 1;
    cnt = cwproccnt[pid];
    cwrunqueue[pid][cnt - 1] = ip;
    cwrunqueue[pid][cnt] = arg1;
    
    return ip;
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
    long            ip;
    long            l;

    cur = cwcurproc[pid];
    ip = cwrunqueue[pid][cur];
    op = &cwoptab[ip];
    fprintf(stderr, "%ld\t%ld\t", pid, ip);
    rcdisasm(op, stderr);
    if (!(*((uint64_t *)op))) {
        if (pid == 0) {
            fprintf(stderr, "program #2 won (%ld)\n", ip);
        } else {
            fprintf(stderr, "program #1 won (%ld)\n", ip);
        }
        
        exit(0);
    }
    func = cwfunctab[op->op];
    ip = func(pid, ip);
    cnt = cwproccnt[pid];
    if (ip == CWNONE) {
        if (cnt > 1) {
            for (l = cur ; l < cnt - 1 ; l++) {
                cwrunqueue[pid][l] = cwrunqueue[pid][l + 1];
            }
        } else {
            if (!pid) {
                fprintf(stderr, "program #2 won\n");
            } else {
                fprintf(stderr, "program #1 won\n");
            }
            
            exit(0);
        }
        cnt--;
        cwproccnt[pid] = cnt;
    } else {
        cwrunqueue[pid][cnt - 1] = ip;
        cur++;
    }
    if (cur == cnt) {
        cur = 0;
    }
    cwcurproc[pid] = cur;
//    rcshowmem();
//    sleep(1);

    return;
}

void
cwloop(void)
{
    long cnt;

    while (1) {
        cwexec(0);
        cwexec(1);
    }
    
    exit(0);
}

int
main(int argc, char *argv[])
{
    FILE *fp;
    long  base;
    long  lim;
    long  ip1;
    long  ip2;
    
    srand(time(NULL));
    cwinitop();
    rcinitop();
    cwoptab = calloc(CWNCORE, sizeof(struct cwinstr));
    if (!cwoptab) {
        fprintf(stderr, "failed to allocate core\n");
        
        exit(1);
    }
    if (argc != 3) {
        fprintf(stderr, "usage: %s prog1.rc prog2.rc\n", argv[0]);
        
        exit(1);
    }
    base = rand() & (CWNCORE - 1);
    fp = fopen(argv[1], "r");
    if (!fp) {
        fprintf(stderr, "failed to open %s\n", argv[1]);
        
        exit(1);
    }
    ip1 = rcxlate(fp, 0, base, &base, &lim);
    if (ip1 < 0) {
        fprintf(stderr, "failed to translate %s\n", argv[1]);

        exit(1);
    }
    fclose(fp);
#if 0
    if (lim < base) {
        base = lim + rand() % ((base - lim) >> 2);
    } else if (lim - base < CWNCORE - lim) {
        base = lim + rand() % ((CWNCORE - lim) >> 2);
    } else {
        base = rand() % (base >> 2);
    }
#endif
    base = rand() & (CWNCORE - 1);
    fp = fopen(argv[2], "r");
    if (!fp) {
        fprintf(stderr, "failed to open %s\n", argv[2]);

        exit(1);
    }
    ip2 = rcxlate(fp, 1, base, &base, &lim);
    if (ip2 < 0) {
        fprintf(stderr, "failed to translate %s\n", argv[1]);

        exit(1);
    }
    fclose(fp);
    cwcurproc[0] = 0;
    cwcurproc[1] = 0;
    cwproccnt[0] = 1;
    cwproccnt[1] = 1;
    cwrunqueue[0][0] = ip1;
    cwrunqueue[1][0] = ip2;
    cwloop();

    /* NOTREACHED */
    exit(0);
}

