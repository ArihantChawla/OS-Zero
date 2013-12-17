/* REFERENCE:  http://corewar.co.uk/cwg.txt */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <zero/param.h>
#include <zero/cdecl.h>

#include <corewar/cw.h>
#include <corewar/rc.h>

typedef long cwinstrfunc(long);

static cwinstrfunc  *cwfunctab[CWNOP];
struct cwinstr     **cwoptab;

/* read n (1 or 2) arguments into arg1 and arg2 */
#define cwreadargs(op, ip, arg1, arg2, n)                               \
    do {                                                                \
        uint64_t _arg1 = ~UINT64_C(0);                                  \
        uint64_t _arg2 = ~UINT64_C(0);                                  \
        uint64_t _tmp;                                                  \
                                                                        \
        if (n == 2) {                                                   \
            if ((op)->aflg & CWIMMBIT) {                                \
                _tmp = (op)->a;                                         \
                _arg1 = (uint64_t)cwoptab[_tmp];                        \
                _arg1 &= CWNCORE - 1;                                   \
            } else if ((op)->aflg & CWINDIRBIT) {                       \
                struct cwinstr *_ptr;                                   \
                                                                        \
                _tmp = ip + (op)->a;                                    \
                _tmp &= CWNCORE - 1;                                    \
                _ptr = cwoptab[_tmp];                                   \
                _tmp += (uint64_t)_ptr;                                 \
                _arg1 = (uint64_t)cwoptab[_tmp];                        \
                _arg1 &= CWNCORE - 1;                                   \
            } else {                                                    \
                _tmp = (ip) + (op)->a;                                  \
                _arg1 = (uint64_t)cwoptab[_tmp];                        \
                _arg1 &= CWNCORE - 1;                                   \
            }                                                           \
        }                                                               \
        if ((op)->bflg & CWIMMBIT) {                                    \
            _tmp = (op)->b;                                             \
            _arg2 = (uint64_t)cwoptab[_tmp];                            \
            _arg2 &= CWNCORE - 1;                                       \
        } else if ((op)->bflg & CWINDIRBIT) {                           \
            struct cwinstr *_ptr;                                       \
                                                                        \
            _tmp = ip + (op)->b;                                        \
            _tmp &= CWNCORE - 1;                                        \
            _ptr = cwoptab[_tmp];                                       \
            _tmp += (uint64_t)_ptr;                                     \
            _arg2 = (uint64_t)cwoptab[_tmp];                            \
            _arg2 &= CWNCORE - 1;                                       \
        } else {                                                        \
            _tmp = (ip) + (op)->b;                                      \
            _arg2 = (uint64_t)cwoptab[_tmp];                            \
            _arg2 &= CWNCORE - 1;                                       \
        }                                                               \
        (arg1) = _arg1;                                                 \
        (arg2) = _arg2;                                                 \
    } while (0)

/* DAT B; initialise location to B */
long
cwdatop(long ip)
{
    struct cwinstr *op = cwoptab[ip];
    uint64_t        dummy;
    uint64_t        val;

    cwreadargs(op, ip, dummy, val, 1);
    cwoptab[ip] = (struct cwinstr *)val;
    ip++;
    ip &= CWNCORE - 1;

    return ip;
}

long
cwmovop(long ip)
{
    struct cwinstr *op = cwoptab[ip];
    uint64_t        src;
    uint64_t        dest;

    if (op->bflg & CWIMMBIT) {
        fprintf(stderr, "MOV: invalid immediate B-field\n");

        exit(1);
    }
    cwreadargs(op, ip, src, dest, 2);
    cwoptab[dest] = (struct cwinstr *)src;
    ip++;
    ip &= CWNCORE - 1;

    return ip;
}

long
cwaddop(long ip)
{
    struct cwinstr *op = cwoptab[ip];
    long            sum;
    uint64_t        src;
    uint64_t        dest;
    long            a;
    long            b;

    if (op->bflg & CWIMMBIT) {
        fprintf(stderr, "ADD: invalid immediate B-field\n");

        exit(1);
    }
    cwreadargs(op, ip, src, dest, 2);
    a = *((long *)&src);
    b = *((long *)&dest);
    sum = src + dest;
    sum &= CWNCORE - 1;
    cwoptab[dest] = (struct cwinstr *)sum;
    ip++;
    ip &= CWNCORE - 1;

    return ip;
}

long
cwsubop(long ip)
{
    struct cwinstr *op = cwoptab[ip];
    long            diff;
    long            dest;
    long            src;
    long            a;
    long            b;

    if (op->bflg & CWIMMBIT) {
        fprintf(stderr, "SUB: invalid immediate B-field\n");

        exit(1);
    }
    cwreadargs(op, ip, src, dest, 2);
    a = *((long *)&src);
    b = *((long *)&dest);
    diff = b - a;
    if (diff < 0) {
        diff += CWNCORE;
    }
    diff &= CWNCORE - 1;
    cwoptab[dest] = (struct cwinstr *)diff;
    ip++;
    ip &= CWNCORE - 1;

    return ip;
}

long
cwjmpop(long ip)
{
    struct cwinstr *op = cwoptab[ip];
    long            dummy;
    long            dest;

    if (op->bflg & CWIMMBIT) {
        fprintf(stderr, "JMP: invalid immediate B-field\n");

        exit(1);
    }
    cwreadargs(op, ip, dummy, dest, 1);
    dest &= CWNCORE - 1;

    return dest;
}

long
cwjmzop(long ip)
{
    struct cwinstr *op = cwoptab[ip];
    long            cond;
    long            dest;

    if (op->bflg & CWIMMBIT) {
        fprintf(stderr, "JMZ: invalid immediate B-field\n");

        exit(1);
    }
    cwreadargs(op, ip, cond, dest, 2);
    if (!cond) {
        ip = dest;
    } else {
        ip++;
    }
    ip &= CWNCORE - 1;

    return ip;
}

long
cwdjzop(long ip)
{
    struct cwinstr *op = cwoptab[ip];
    long            src;
    long            dest;
 
    if (op->bflg & CWIMMBIT) {
        fprintf(stderr, "JZ: invalid immediate B-field\n");
 
        exit(1);
    }
    cwreadargs(op, ip, src, dest, 2);
    src--;
    if (src < 0) {
        src = CWNCORE - 1;
        ip++;
    } else if (!src) {
        ip = dest;
    } else {
        ip++;
    }
    ip &= CWNCORE - 1;

    return ip;
}

long
cwcmpop(long ip)
{
    struct cwinstr *op = cwoptab[ip];
    long            src;
    long            dest;

    cwreadargs(op, ip, src, dest, 2);
    if (src != dest) {
        ip += 2;
    } else {
        ip++;
    }
    ip &= CWNCORE - 1;

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
    cwfunctab[CWOPDJZ] = cwdjzop;
    cwfunctab[CWOPCMP] = cwcmpop;
    rcaddop("DAT", CWOPDAT);
    rcaddop("MOV", CWOPMOV);
    rcaddop("ADD", CWOPADD);
    rcaddop("SUB", CWOPSUB);
    rcaddop("JMP", CWOPJMP);
    rcaddop("JMZ", CWOPJMZ);
    rcaddop("DJZ", CWOPDJZ);
    rcaddop("CMP", CWOPCMP);
}

void
cwloop(long start1, long start2)
{
    long            ip1 = start1;
    long            ip2 = start2;
    struct cwinstr *op;
    cwinstrfunc    *func;

    while (1) {
        op = cwoptab[ip1];
        if (!op) {
            fprintf(stderr, "program #2 won\n");

            exit(0);
        }
        func = cwfunctab[op->op];
        ip1 = func(ip1);
        op = cwoptab[ip2];
        if (!op) {
            fprintf(stderr, "program #1 won\n");

            exit(0);
        }
        func = cwfunctab[op->op];
        ip2 = func(ip2);
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

    cwinitop();
    cwoptab = calloc(CWNCORE, sizeof(struct cwinstr *));
    if (!cwoptab) {
        fprintf(stderr, "failed to allocate core\n");

        exit(1);
    }
    if (argc != 3) {
        fprintf(stderr, "usage: %s prog1.rc prog2.rc\n", argv[0]);

        exit(1);
    }
    fp = fopen(argv[1], "r");
    if (!fp) {
        fprintf(stderr, "failed to open %s\n", argv[1]);

        exit(1);
    }
    if (!rcxlate(fp, rand() & (CWNCORE - 1), &base, &lim)) {
        fprintf(stderr, "failed to translate %s\n", argv[1]);

        exit(1);
    }
    ip1 = base;
    if (lim < base) {
        base = lim + ((base - lim) >> 1);
    } else if (base < CWNCORE - lim) {
        base = lim + ((CWNCORE - lim) >> 1);
    } else {
        base += ((lim - base) >> 1);
    }
    fp = fopen(argv[2], "r");
    if (!fp) {
        fprintf(stderr, "failed to open %s\n", argv[2]);

        exit(1);
    }
    if (!rcxlate(fp, base, &base, &lim)) {
        fprintf(stderr, "failed to translate %s\n", argv[1]);

        exit(1);
    }
    ip2 = base;
    cwloop(ip1, ip2);

    /* NOTREACHED */
    exit(0);
}

