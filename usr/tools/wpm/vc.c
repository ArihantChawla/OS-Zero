#include <wpm/conf.h>

#if (WPMVEC)

#include <stddef.h>
#if (WPMPROF)
#include <zero/prof.h>
#endif
#include <wpm/wpm.h>
#include <wpm/vc.h>

extern __thread struct wpm *wpm;

void
opvadd(struct wpmopcode *op)
{
    struct wpmvecopcode *vop = (struct wpmvecopcode *)op;
    long                 reg1 = vop->reg1;
    long                 reg2 = vop->reg2;
    wpmmemadr_t          adr1 = wpm->cpustat.varegs[reg1];
    wpmmemadr_t          adr2 = wpm->cpustat.varegs[reg2];
    uint64_t             len1 = wpm->cpustat.vlregs[reg1];
    uint64_t             len2 = wpm->cpustat.vlregs[reg2];
#if (WPMPROF)
    PROFDECLTICK(tick);
#endif
    
#if (WPMPROF)
    profstarttick(tick);
#endif
    switch (wpmvecoptype(vop)) {
        case OP_FLOAT:
            vcfloatop2(adr1, adr2, len1, len2, vcadd);

            break;
        case OP_BYTE:
            vcintop2b(adr1, adr2, len1, len2, vcadd, wpmvecopflg(vop));

            break;
        case OP_WORD:
            vcintop2w(adr1, adr2, len1, len2, vcadd, wpmvecopflg(vop));

            break;
        case OP_LONG:
            vcintop2l(adr1, adr2, len1, len2, vcadd, wpmvecopflg(vop));

            break;
        case OP_QUAD:
            vcintop2q(adr1, adr2, len1, len2, vcadd, wpmvecopflg(vop));

            break;
}
#if (WPMPROF)
    profstoptick(tick);
    fprintf(stderr, "VADD: %lld cycles\n", (long long)proftickdiff(tick));
#endif

    wpm->cpustat.pc += 4 + (vop->narg << 2);
}

void
opvsub(struct wpmopcode *op)
{
}

void
opvmul(struct wpmopcode *op)
{
}

void
opvdiv(struct wpmopcode *op)
{
}

void
opvmod(struct wpmopcode *op)
{
}

void
opvlt(struct wpmopcode *op)
{
}

void
opvlte(struct wpmopcode *op)
{
}

void
opvgt(struct wpmopcode *op)
{
    return;
}

void
opvgte(struct wpmopcode *op)
{
    return;
}

void
opveq(struct wpmopcode *op)
{
    return;
}

void
opvineq(struct wpmopcode *op)
{
    return;
}

void
opvshl(struct wpmopcode *op)
{
    return;
}

void
opvshr(struct wpmopcode *op)
{
    return;
}

void
opvnot(struct wpmopcode *op)
{
    return;
}

void
opvand(struct wpmopcode *op)
{
    return;
}

void
opvor(struct wpmopcode *op)
{
    return;
}

void
opvxor(struct wpmopcode *op)
{
    return;
}

/* TODO: three arguments */
void
opvselect(struct wpmopcode *op)
{
    return;
}

void
opvrand(struct wpmopcode *op)
{
    return;
}

void
opvfloor(struct wpmopcode *op)
{
    return;
}

void
opvceil(struct wpmopcode *op)
{
    return;
}

void
opvtrunc(struct wpmopcode *op)
{
    return;
}

void
opvround(struct wpmopcode *op)
{
    return;
}

void
opvitof(struct wpmopcode *op)
{
    return;
}

void
opvitob(struct wpmopcode *op)
{
    return;
}

void
opvbtoi(struct wpmopcode *op)
{
    return;
}

void
opvlog(struct wpmopcode *op)
{
    return;
}

void
opvsqrt(struct wpmopcode *op)
{
    return;
}

void
opvexp(struct wpmopcode *op)
{
    return;
}

void
opvsin(struct wpmopcode *op)
{
}

void
opvcos(struct wpmopcode *op)
{
    return;
}

void
opvtan(struct wpmopcode *op)
{
    return;
}

void
opvasin(struct wpmopcode *op)
{
    return;
}

void
opvacos(struct wpmopcode *op)
{
    return;
}

void
opvatan(struct wpmopcode *op)
{
    return;
}

void
opvsinh(struct wpmopcode *op)
{
    return;
}

void
opvcosh(struct wpmopcode *op)
{
    return;
}

void
opvtanh(struct wpmopcode *op)
{
    return;
}

void
opvplscan(struct wpmopcode *op)
{
    return;
}

void
opvmulscan(struct wpmopcode *op)
{
    return;
}

void
opvmaxscan(struct wpmopcode *op)
{
    return;
}

void
opvminscan(struct wpmopcode *op)
{
    return;
}

void
opvandscan(struct wpmopcode *op)
{
    return;
}

void
opvorscan(struct wpmopcode *op)
{
    return;
}

void
opvxorscan(struct wpmopcode *op)
{
    return;
}

void
opvplreduce(struct wpmopcode *op)
{
    return;
}

void
opvmulreduce(struct wpmopcode *op)
{
    return;
}

void
opvmaxreduce(struct wpmopcode *op)
{
    return;
}

void
opvminreduce(struct wpmopcode *op)
{
    return;
}
void
opvandreduce(struct wpmopcode *op)

{
    return;
}

void
opvorreduce(struct wpmopcode *op)
{
    return;
}

void
opvxorreduce(struct wpmopcode *op)
{
    return;
}

void
opvpermute(struct wpmopcode *op)
{
    return;
}

void
opvdpermute(struct wpmopcode *op)
{
    return;
}

void
opvfpermute(struct wpmopcode *op)
{
    return;
}

void
opvbpermute(struct wpmopcode *op)
{
    return;
}

void
opvbfpermute(struct wpmopcode *op)
{
    return;
}

void
opvdfpermute(struct wpmopcode *op)
{
    return;
}

void
opvextract(struct wpmopcode *op)
{
    return;
}

void
opvreplace(struct wpmopcode *op)
{
    return;
}

void
opvpack(struct wpmopcode *op)
{
    return;
}

void
opvrankup(struct wpmopcode *op)
{
    return;
}

void
opvrankdown(struct wpmopcode *op)
{
    return;
}

void
opvdist(struct wpmopcode *op)
{
    return;
}

void
opvindex(struct wpmopcode *op)
{
    return;
}

void
opvlength(struct wpmopcode *op)
{
    return;
}

void
opvmkdes(struct wpmopcode *op)
{
    return;
}

void
opvlengths(struct wpmopcode *op)
{
    return;
}

void
opvcopy(struct wpmopcode *op)
{
    return;
}

void
opvpop(struct wpmopcode *op)
{
    return;
}

void
opvcpop(struct wpmopcode *op)
{
    return;
}

void
opvpair(struct wpmopcode *op)
{
    return;
}

void
opvunpair(struct wpmopcode *op)
{
    return;
}

#endif /* WPMVEC */

