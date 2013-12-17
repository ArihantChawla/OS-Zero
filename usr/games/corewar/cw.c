/* REFERENCE:  http://corewar.co.uk/cwg.txt */

#include <stdio.h>
#include <stdlib.h>
#include <zero/param.h>
#include <zero/cdecl.h>

#include <corewar/cw.h>

typedef long cwinstrfunc(long);

static struct cwinstr  cwcore[CWNCORE];
static long            cwcoretab[CWNCORE];
static struct cwinstr *cwoptab[CWNCORE];
static cwinstrfunc    *cwfunctab[CWNOP];

#define cwreadargs(op, arg1, arg2)                                      \
    do {                                                                \
        (arg1) = CWNONE;                                                \
        (arg2) = CWNONE;                                                \
        if ((op)->aflg & CWRELBIT) {                                    \
            (arg1) = ip + (op)->a;                                      \
        } else if ((op)->aflg & CWIMMBIT) {                             \
            (arg1) = (op)->a;                                           \
        } else if ((op)->aflg & CWINDIRBIT) {                           \
            long *lp = &cwcoretab[ip];                                  \
            (arg1) = lp[(op)->a];                                       \
        }                                                               \
        if ((op)->bflg & CWRELBIT) {                                    \
            (arg2) = ip + (op)->b;                                      \
        } else if ((op)->bflg & CWIMMBIT) {                             \
            (arg2) = (op)->b;                                           \
        } else if ((op)->bflg & CWINDIRBIT) {                           \
            long *lp = &cwcoretab[ip];                                  \
            (arg2) = lp[(op)->b];                                       \
        }                                                               \
        (arg2) &= CWNCORE - 1;                                          \
        (arg1) &= CWNCORE - 1;                                          \
    } while (0)

long
cwdatop(long ip)
{
    struct cwinstr *op = cwoptab[ip];
    long            ret = op->b;

    return ret;
}

long
cwmovop(long ip)
{
    struct cwinstr *op = cwoptab[ip];
    long            ret = ip;
    long            dest;
    long            src;

    cwreadargs(op, src, dest);
    cwcoretab[dest] = cwcoretab[src];
    cwoptab[dest] = cwoptab[src];
    /* FIXME: does cwoptab[src] become empty/NULL? */
    cwoptab[src] = NULL;
    ret++;
    ret &= CWNCORE - 1;

    return ret;
}

long
cwaddop(long ip)
{
    struct cwinstr *op = cwoptab[ip];
    long            ret = ip;
    long            sum;
    long            dest;
    long            src;

    cwreadargs(op, src, dest);
    sum = cwcoretab[src] + cwcoretab[dest];
    sum &= CWNCORE - 1;
    cwcoretab[dest] = sum;
    cwoptab[dest] = cwoptab[sum];
    ret++;
    ret &= CWNCORE - 1;

    return ret;
}

long
cwsubop(long ip)
{
    struct cwinstr *op = cwoptab[ip];
    long            ret = ip;
    long            diff;
    long            dest;
    long            src;

    cwreadargs(op, src, dest);
    diff = cwcoretab[src] - cwcoretab[dest];
    diff &= CWNCORE - 1;
    cwcoretab[dest] = diff;
    cwoptab[dest] = cwoptab[diff];
    ret++;
    ret &= CWNCORE - 1;

    return ret;
}

long
cwjmpop(long ip)
{
    struct cwinstr *op = cwoptab[ip];
    long            dummy;
    long            dest;

    cwreadargs(op, dummy, dest);
    dest &= CWNCORE - 1;

    return dest;
}

long
cwjmzop(long ip)
{
    struct cwinstr *op = cwoptab[ip];
    long            ret = ip;
    long            cond;
    long            dest;

    cwreadargs(op, cond, dest);
    if (!cond) {
        ret = dest;
    } else {
        ret++;
        ret &= CWNCORE - 1;
    }

    return ret;
}

long
cwdjzop(long ip)
{
    struct cwinstr *op = cwoptab[ip];
    long            ret = ip;
    long            src;
    long            dest;

    cwreadargs(op, src, dest);
    src--;
    if (src < 0) {
        src = CWNCORE - 1;
    } else if (!src) {
        ret = dest;
    } else {
        ret++;
        ret &= CWNCORE - 1;
    }

    return ret;
}

long
cwcmpop(long ip)
{
    struct cwinstr *op = cwoptab[ip];
    long            ret = ip;
    long            src;
    long            dest;

    cwreadargs(op, src, dest);
    if (src != dest) {
        ret += 2;
    } else {
        ret++;
    }
    ret &= CWNCORE - 1;

    return ret;
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
cwinitcore(void)
{
    long l;

    for (l = 0 ; l < CWNCORE ; l++) {
        cwoptab[l] = &cwcore[l];
    }

    return;
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
    cwinitcore();

    exit(0);
}
