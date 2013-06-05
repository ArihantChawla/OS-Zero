#include <wpm/conf.h>

#if (WPMVEC)

#include <stddef.h>
#if (WPMPROF)
#include <zero/prof.h>
#endif
#include <wpm/wpm.h>
#include <wpm/vc.h>

extern __thread struct wpm *wpm;

#define wpmvecop1(vop, OP)                                              \
    do {                                                                \
        long                 reg1 = vop->reg1;                          \
        long                 reg2 = vop->reg2;                          \
        wpmmemadr_t          adr1 = wpm->cpustat.varegs[reg1];          \
        wpmmemadr_t          adr2 = wpm->cpustat.varegs[reg2];          \
        uint64_t             len1 = wpm->cpustat.vlregs[reg1];          \
        uint64_t             len2 = wpm->cpustat.vlregs[reg2];          \
                                                                        \
        switch (wpmvecoptype(vop)) {                                    \
            case OP_FLOAT:                                              \
                vcfloatop1(adr1, adr2, len1, len2, OP);                 \
                                                                        \
                break;                                                  \
            case OP_BYTE:                                               \
                vcintop1b(adr1, adr2, len1, len2, OP, wpmvecopflg(vop)); \
                                                                        \
                break;                                                  \
            case OP_WORD:                                               \
                vcintop1w(adr1, adr2, len1, len2, OP, wpmvecopflg(vop)); \
                                                                        \
                break;                                                  \
            case OP_LONG:                                               \
                vcintop1l(adr1, adr2, len1, len2, OP, wpmvecopflg(vop)); \
                                                                        \
                break;                                                  \
            case OP_QUAD:                                               \
                vcintop1q(adr1, adr2, len1, len2, OP, wpmvecopflg(vop)); \
                                                                        \
                break;                                                  \
        }                                                               \
    } while (0)

#define wpmvecfloatop1(vop, OP)                                         \
    do {                                                                \
        long                 reg1 = vop->reg1;                          \
        long                 reg2 = vop->reg2;                          \
        wpmmemadr_t          adr1 = wpm->cpustat.varegs[reg1];          \
        wpmmemadr_t          adr2 = wpm->cpustat.varegs[reg2];          \
        uint64_t             len1 = wpm->cpustat.vlregs[reg1];          \
        uint64_t             len2 = wpm->cpustat.vlregs[reg2];          \
                                                                        \
        vcfloatop1(adr1, len1, OP);                                     \
    } while (0)

#define wpmvecintop1(vop, OP)                                           \
    do {                                                                \
        long                 reg1 = vop->reg1;                          \
        wpmmemadr_t          adr1 = wpm->cpustat.varegs[reg1];          \
        uint64_t             len1 = wpm->cpustat.vlregs[reg1];          \
                                                                        \
        switch (wpmvecoptype(vop)) {                                    \
            case OP_BYTE:                                               \
                vcintop1b(adr1, len1, OP);                              \
                                                                        \
                break;                                                  \
            case OP_WORD:                                               \
                vcintop1w(adr1, len1, OP);                              \
                                                                        \
                break;                                                  \
            case OP_LONG:                                               \
                vcintop1l(adr1, len1, OP);                              \
                                                                        \
                break;                                                  \
            case OP_QUAD:                                               \
                vcintop1q(adr1, len1, OP);                              \
                                                                        \
                break;                                                  \
        }                                                               \
    } while (0)

#define wpmvecop2(vop, OP)                                              \
    do {                                                                \
        long                 reg1 = vop->reg1;                          \
        long                 reg2 = vop->reg2;                          \
        wpmmemadr_t          adr1 = wpm->cpustat.varegs[reg1];          \
        wpmmemadr_t          adr2 = wpm->cpustat.varegs[reg2];          \
        uint64_t             len1 = wpm->cpustat.vlregs[reg1];          \
        uint64_t             len2 = wpm->cpustat.vlregs[reg2];          \
                                                                        \
        switch (wpmvecoptype(vop)) {                                    \
            case OP_FLOAT:                                              \
                vcfloatop2(adr1, adr2, len1, len2, OP);                 \
                                                                        \
                break;                                                  \
            case OP_BYTE:                                               \
                vcintop2b(adr1, adr2, len1, len2, OP, wpmvecopflg(vop)); \
                                                                        \
                break;                                                  \
            case OP_WORD:                                               \
                vcintop2w(adr1, adr2, len1, len2, OP, wpmvecopflg(vop)); \
                                                                        \
                break;                                                  \
            case OP_LONG:                                               \
                vcintop2l(adr1, adr2, len1, len2, OP, wpmvecopflg(vop)); \
                                                                        \
                break;                                                  \
            case OP_QUAD:                                               \
                vcintop2q(adr1, adr2, len1, len2, OP, wpmvecopflg(vop)); \
                                                                        \
                break;                                                  \
        }                                                               \
    } while (0)

#define wpmvecintop2(vop, OP)                                           \
    do {                                                                \
        long                 reg1 = vop->reg1;                          \
        long                 reg2 = vop->reg2;                          \
        wpmmemadr_t          adr1 = wpm->cpustat.varegs[reg1];          \
        wpmmemadr_t          adr2 = wpm->cpustat.varegs[reg2];          \
        uint64_t             len1 = wpm->cpustat.vlregs[reg1];          \
        uint64_t             len2 = wpm->cpustat.vlregs[reg2];          \
                                                                        \
        switch (wpmvecoptype(vop)) {                                    \
            case OP_BYTE:                                               \
                vcintop2b(adr1, adr2, len1, len2, OP, wpmvecopflg(vop)); \
                                                                        \
                break;                                                  \
            case OP_WORD:                                               \
                vcintop2w(adr1, adr2, len1, len2, OP, wpmvecopflg(vop)); \
                                                                        \
                break;                                                  \
            case OP_LONG:                                               \
                vcintop2l(adr1, adr2, len1, len2, OP, wpmvecopflg(vop)); \
                                                                        \
                break;                                                  \
            case OP_QUAD:                                               \
                vcintop2q(adr1, adr2, len1, len2, OP, wpmvecopflg(vop)); \
                                                                        \
                break;                                                  \
        }                                                               \
    } while (0)

void
opvadd(struct wpmopcode *op)
{
    struct wpmvecopcode *vop = (struct wpmvecopcode *)op;
    
    wpmvecop2(vop, vcadd);
    wpm->cpustat.pc += 4 + (vop->narg << 2);
}

void
opvsub(struct wpmopcode *op)
{
    struct wpmvecopcode *vop = (struct wpmvecopcode *)op;

    wpmvecop2(vop, vcsub);
    wpm->cpustat.pc += 4 + (vop->narg << 2);

    return;
}

void
opvmul(struct wpmopcode *op)
{
    struct wpmvecopcode *vop = (struct wpmvecopcode *)op;

    wpmvecop2(vop, vcmul);
    wpm->cpustat.pc += 4 + (vop->narg << 2);

    return;
}

void
opvdiv(struct wpmopcode *op)
{
    struct wpmvecopcode *vop = (struct wpmvecopcode *)op;

    wpmvecop2(vop, vcdiv);
    wpm->cpustat.pc += 4 + (vop->narg << 2);

    return;
}

void
opvmod(struct wpmopcode *op)
{
    struct wpmvecopcode *vop = (struct wpmvecopcode *)op;

    wpmvecintop2(vop, vcmod);
    wpm->cpustat.pc += 4 + (vop->narg << 2);

    return;
}

void
opvlt(struct wpmopcode *op)
{
    struct wpmvecopcode *vop = (struct wpmvecopcode *)op;

    wpmvecop2(vop, vclt);
    wpm->cpustat.pc += 4 + (vop->narg << 2);

    return;
}

void
opvlte(struct wpmopcode *op)
{
    struct wpmvecopcode *vop = (struct wpmvecopcode *)op;

    wpmvecop2(vop, vclte);
    wpm->cpustat.pc += 4 + (vop->narg << 2);

    return;
}

void
opvgt(struct wpmopcode *op)
{
    struct wpmvecopcode *vop = (struct wpmvecopcode *)op;

    wpmvecop2(vop, vcgt);
    wpm->cpustat.pc += 4 + (vop->narg << 2);

    return;
}

void
opvgte(struct wpmopcode *op)
{
    struct wpmvecopcode *vop = (struct wpmvecopcode *)op;

    wpmvecop2(vop, vcgte);
    wpm->cpustat.pc += 4 + (vop->narg << 2);

    return;
}

void
opveq(struct wpmopcode *op)
{
    struct wpmvecopcode *vop = (struct wpmvecopcode *)op;

    wpmvecop2(vop, vceq);
    wpm->cpustat.pc += 4 + (vop->narg << 2);

    return;
}

void
opvineq(struct wpmopcode *op)
{
    struct wpmvecopcode *vop = (struct wpmvecopcode *)op;

    wpmvecop2(vop, vcineq);
    wpm->cpustat.pc += 4 + (vop->narg << 2);

    return;
}

void
opvshl(struct wpmopcode *op)
{
    struct wpmvecopcode *vop = (struct wpmvecopcode *)op;

    wpmvecintop2(vop, vcshl);
    wpm->cpustat.pc += 4 + (vop->narg << 2);

    return;
}

void
opvshr(struct wpmopcode *op)
{
    struct wpmvecopcode *vop = (struct wpmvecopcode *)op;

    wpmvecintop2(vop, vcshr);
    wpm->cpustat.pc += 4 + (vop->narg << 2);

    return;
}

void
opvnot(struct wpmopcode *op)
{
    struct wpmvecopcode *vop = (struct wpmvecopcode *)op;

    wpmvecintop1(vop, vcnot);
    wpm->cpustat.pc += 4 + (vop->narg << 2);

    return;
}

void
opvand(struct wpmopcode *op)
{
    struct wpmvecopcode *vop = (struct wpmvecopcode *)op;

    wpmvecintop2(vop, vcand);
    wpm->cpustat.pc += 4 + (vop->narg << 2);

    return;
}

void
opvor(struct wpmopcode *op)
{
    struct wpmvecopcode *vop = (struct wpmvecopcode *)op;

    wpmvecintop2(vop, vcor);
    wpm->cpustat.pc += 4 + (vop->narg << 2);

    return;
}

void
opvxor(struct wpmopcode *op)
{
    struct wpmvecopcode *vop = (struct wpmvecopcode *)op;

    wpmvecintop2(vop, vcxor);
    wpm->cpustat.pc += 4 + (vop->narg << 2);

    return;
}

/* TODO: three arguments */
void
opvselect(struct wpmopcode *op)
{
    struct wpmvecopcode *vop = (struct wpmvecopcode *)op;

//    wpmvecop2(vop, vcselect);

    return;
}

void
opvrand(struct wpmopcode *op)
{
    struct wpmvecopcode *vop = (struct wpmvecopcode *)op;

    wpmvecintop1(vop, vcrand);
    wpm->cpustat.pc += 4 + (vop->narg << 2);

    return;
}

void
opvfloor(struct wpmopcode *op)
{
    struct wpmvecopcode *vop = (struct wpmvecopcode *)op;

    wpmvecfloatop1(vop, vcfloor);
    wpm->cpustat.pc += 4 + (vop->narg << 2);

    return;
}

void
opvceil(struct wpmopcode *op)
{
    struct wpmvecopcode *vop = (struct wpmvecopcode *)op;

    wpmvecfloatop1(vop, vcceil);
    wpm->cpustat.pc += 4 + (vop->narg << 2);

    return;
}

void
opvtrunc(struct wpmopcode *op)
{
    struct wpmvecopcode *vop = (struct wpmvecopcode *)op;

    return;
}

void
opvround(struct wpmopcode *op)
{
    struct wpmvecopcode *vop = (struct wpmvecopcode *)op;

    return;
}

void
opvitof(struct wpmopcode *op)
{
    struct wpmvecopcode *vop = (struct wpmvecopcode *)op;

    return;
}

void
opvitob(struct wpmopcode *op)
{
    struct wpmvecopcode *vop = (struct wpmvecopcode *)op;

    return;
}

void
opvbtoi(struct wpmopcode *op)
{
    struct wpmvecopcode *vop = (struct wpmvecopcode *)op;

    return;
}

void
opvlog(struct wpmopcode *op)
{
    struct wpmvecopcode *vop = (struct wpmvecopcode *)op;

    return;
}

void
opvsqrt(struct wpmopcode *op)
{
    struct wpmvecopcode *vop = (struct wpmvecopcode *)op;

    return;
}

void
opvexp(struct wpmopcode *op)
{
    struct wpmvecopcode *vop = (struct wpmvecopcode *)op;

    return;
}

void
opvsin(struct wpmopcode *op)
{
    struct wpmvecopcode *vop = (struct wpmvecopcode *)op;
}

void
opvcos(struct wpmopcode *op)
{
    struct wpmvecopcode *vop = (struct wpmvecopcode *)op;

    return;
}

void
opvtan(struct wpmopcode *op)
{
    struct wpmvecopcode *vop = (struct wpmvecopcode *)op;

    return;
}

void
opvasin(struct wpmopcode *op)
{
    struct wpmvecopcode *vop = (struct wpmvecopcode *)op;

    return;
}

void
opvacos(struct wpmopcode *op)
{
    struct wpmvecopcode *vop = (struct wpmvecopcode *)op;

    return;
}

void
opvatan(struct wpmopcode *op)
{
    struct wpmvecopcode *vop = (struct wpmvecopcode *)op;

    return;
}

void
opvsinh(struct wpmopcode *op)
{
    struct wpmvecopcode *vop = (struct wpmvecopcode *)op;

    return;
}

void
opvcosh(struct wpmopcode *op)
{
    struct wpmvecopcode *vop = (struct wpmvecopcode *)op;

    return;
}

void
opvtanh(struct wpmopcode *op)
{
    struct wpmvecopcode *vop = (struct wpmvecopcode *)op;

    return;
}

#if (WPMVECFULL)

void
opvplscan(struct wpmopcode *op)
{
    struct wpmvecopcode *vop = (struct wpmvecopcode *)op;

    return;
}

void
opvmulscan(struct wpmopcode *op)
{
    struct wpmvecopcode *vop = (struct wpmvecopcode *)op;

    return;
}

void
opvmaxscan(struct wpmopcode *op)
{
    struct wpmvecopcode *vop = (struct wpmvecopcode *)op;

    return;
}

void
opvminscan(struct wpmopcode *op)
{
    struct wpmvecopcode *vop = (struct wpmvecopcode *)op;

    return;
}

void
opvandscan(struct wpmopcode *op)
{
    struct wpmvecopcode *vop = (struct wpmvecopcode *)op;

    return;
}

void
opvorscan(struct wpmopcode *op)
{
    struct wpmvecopcode *vop = (struct wpmvecopcode *)op;

    return;
}

void
opvxorscan(struct wpmopcode *op)
{
    struct wpmvecopcode *vop = (struct wpmvecopcode *)op;

    return;
}

void
opvplreduce(struct wpmopcode *op)
{
    struct wpmvecopcode *vop = (struct wpmvecopcode *)op;

    return;
}

void
opvmulreduce(struct wpmopcode *op)
{
    struct wpmvecopcode *vop = (struct wpmvecopcode *)op;

    return;
}

void
opvmaxreduce(struct wpmopcode *op)
{
    struct wpmvecopcode *vop = (struct wpmvecopcode *)op;

    return;
}

void
opvminreduce(struct wpmopcode *op)
{
    struct wpmvecopcode *vop = (struct wpmvecopcode *)op;

    return;
}
void
opvandreduce(struct wpmopcode *op)

{
    struct wpmvecopcode *vop = (struct wpmvecopcode *)op;

    return;
}

void
opvorreduce(struct wpmopcode *op)
{
    struct wpmvecopcode *vop = (struct wpmvecopcode *)op;

    return;
}

void
opvxorreduce(struct wpmopcode *op)
{
    struct wpmvecopcode *vop = (struct wpmvecopcode *)op;

    return;
}

void
opvpermute(struct wpmopcode *op)
{
    struct wpmvecopcode *vop = (struct wpmvecopcode *)op;

    return;
}

void
opvdpermute(struct wpmopcode *op)
{
    struct wpmvecopcode *vop = (struct wpmvecopcode *)op;

    return;
}

void
opvfpermute(struct wpmopcode *op)
{
    struct wpmvecopcode *vop = (struct wpmvecopcode *)op;

    return;
}

void
opvbpermute(struct wpmopcode *op)
{
    struct wpmvecopcode *vop = (struct wpmvecopcode *)op;

    return;
}

void
opvbfpermute(struct wpmopcode *op)
{
    struct wpmvecopcode *vop = (struct wpmvecopcode *)op;

    return;
}

void
opvdfpermute(struct wpmopcode *op)
{
    struct wpmvecopcode *vop = (struct wpmvecopcode *)op;

    return;
}

void
opvextract(struct wpmopcode *op)
{
    struct wpmvecopcode *vop = (struct wpmvecopcode *)op;

    return;
}

void
opvreplace(struct wpmopcode *op)
{
    struct wpmvecopcode *vop = (struct wpmvecopcode *)op;

    return;
}

void
opvpack(struct wpmopcode *op)
{
    struct wpmvecopcode *vop = (struct wpmvecopcode *)op;

    return;
}

void
opvrankup(struct wpmopcode *op)
{
    struct wpmvecopcode *vop = (struct wpmvecopcode *)op;

    return;
}

void
opvrankdown(struct wpmopcode *op)
{
    struct wpmvecopcode *vop = (struct wpmvecopcode *)op;

    return;
}

void
opvdist(struct wpmopcode *op)
{
    struct wpmvecopcode *vop = (struct wpmvecopcode *)op;

    return;
}

void
opvindex(struct wpmopcode *op)
{
    struct wpmvecopcode *vop = (struct wpmvecopcode *)op;

    return;
}

void
opvlength(struct wpmopcode *op)
{
    struct wpmvecopcode *vop = (struct wpmvecopcode *)op;

    return;
}

void
opvmkdes(struct wpmopcode *op)
{
    struct wpmvecopcode *vop = (struct wpmvecopcode *)op;

    return;
}

void
opvlengths(struct wpmopcode *op)
{
    struct wpmvecopcode *vop = (struct wpmvecopcode *)op;

    return;
}

void
opvcopy(struct wpmopcode *op)
{
    struct wpmvecopcode *vop = (struct wpmvecopcode *)op;

    return;
}

void
opvpop(struct wpmopcode *op)
{
    struct wpmvecopcode *vop = (struct wpmvecopcode *)op;

    return;
}

void
opvcpop(struct wpmopcode *op)
{
    struct wpmvecopcode *vop = (struct wpmvecopcode *)op;

    return;
}

void
opvpair(struct wpmopcode *op)
{
    struct wpmvecopcode *vop = (struct wpmvecopcode *)op;

    return;
}

void
opvunpair(struct wpmopcode *op)
{
    struct wpmvecopcode *vop = (struct wpmvecopcode *)op;

    return;
}

#endif /* WPMVECFULL */

#endif /* WPMVEC */

