/* REFERENCE:  http://corewar.co.uk/cwg.txt */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <zero/param.h>
#include <zero/cdecl.h>

#include <corewar/cw.h>

typedef long cwinstrfunc(long);

static struct cwinstr *cwoptab[CWNCORE];
static cwinstrfunc    *cwfunctab[CWNOP];

/* read n (1 or 2) arguments into arg1 and arg2 */
#define cwreadargs(op, ip, arg1, arg2, n)                               \
    do {                                                                \
        uintptr_t _arg1 = CWNONE;                                       \
        uintptr_t _arg2 = CWNONE;                                       \
        uintptr_t _tmp;                                                 \
                                                                        \
        (arg1) = CWNONE;                                                \
        (arg2) = CWNONE;                                                \
        if (n == 2) {                                                   \
            if ((op)->aflg & CWIMMBIT) {                                \
                _tmp = (op)->a;                                         \
                _arg1 = (uintptr_t)cwoptab[_tmp];                       \
                _arg1 &= CWNCORE - 1;                                   \
            } else if ((op)->aflg & CWINDIRBIT) {                       \
                struct cwinstr *_ptr;                                   \
                                                                        \
                _tmp = ip + (op)->a;                                    \
                _tmp &= CWNCORE - 1;                                    \
                _ptr = cwoptab[_tmp];                                   \
                _tmp += (uintptr_t)_ptr;                                \
                _arg1 = (uintptr_t)cwoptab[_tmp];                       \
                _arg1 &= CWNCORE - 1;                                   \
            } else {                                                    \
                _tmp = (ip) + (op)->a;                                  \
                _arg1 = (uintptr_t)cwoptab[_tmp];                       \
                _arg1 &= CWNCORE - 1;                                   \
            }                                                           \
        }                                                               \
        if ((op)->bflg & CWIMMBIT) {                                    \
            _tmp = (op)->b;                                             \
            _arg2 = (uintptr_t)cwoptab[_tmp];                           \
            _arg2 &= CWNCORE - 1;                                       \
        } else if ((op)->bflg & CWINDIRBIT) {                           \
            struct cwinstr *_ptr;                                       \
                                                                        \
            _tmp = ip + (op)->b;                                        \
            _tmp &= CWNCORE - 1;                                        \
            _ptr = cwoptab[_tmp];                                       \
            _tmp += (uintptr_t)_ptr;                                    \
            _arg2 = (uintptr_t)cwoptab[_tmp];                           \
            _arg2 &= CWNCORE - 1;                                       \
        } else {                                                        \
            _tmp = (ip) + (op)->b;                                      \
            _arg2 = (uintptr_t)cwoptab[_tmp];                           \
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
    uintptr_t       dummy;
    uintptr_t       val;

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
    uintptr_t       src;
    uintptr_t       dest;

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
    uintptr_t       src;
    uintptr_t       dest;
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
}

void
cwloop(long start)
{
    long            ip = start;
    struct cwinstr *op;
    cwinstrfunc    *func;

    while (1) {
        op = cwoptab[ip];
        func = cwfunctab[op->op];
        ip = func(ip);
    }
}

int
main(int argc, char *argv[])
{
    cwinitop();

    exit(0);
}

