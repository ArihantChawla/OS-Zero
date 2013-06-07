#include <wpm/conf.h>

#if (WPMVEC)

#include <stddef.h>
#if (WPMPROF)
#include <zero/prof.h>
#endif
#include <wpm/wpm.h>
#include <vec/vec.h>
#include <vec/vm.h>

extern __thread struct wpm *wpm;

void
vecopadd(struct wpmopcode *op)
{
    struct vecopcode *vop = (struct vecopcode *)op;
#if (WPMPROF)
    PROFDECLTICK(tick);
#endif
    
#if (WPMPROF)
    profstarttick(tick);
#endif
    vecvmop2(vop, vecadd);
#if (WPMPROF)
    profstoptick(tick);
    fprintf(stderr, "CYCLES: %lld\n", (long long)proftickdiff(tick));
#endif
    wpm->cpustat.pc += 4 + (vop->narg << 2);
}

void
vecopsub(struct wpmopcode *op)
{
    struct vecopcode *vop = (struct vecopcode *)op;

    vecvmop2(vop, vecsub);
    wpm->cpustat.pc += 4 + (vop->narg << 2);

    return;
}

void
vecopmul(struct wpmopcode *op)
{
    struct vecopcode *vop = (struct vecopcode *)op;

    vecvmop2(vop, vecmul);
    wpm->cpustat.pc += 4 + (vop->narg << 2);

    return;
}

void
vecopdiv(struct wpmopcode *op)
{
    struct vecopcode *vop = (struct vecopcode *)op;

    vecvmop2(vop, vecdiv);
    wpm->cpustat.pc += 4 + (vop->narg << 2);

    return;
}

void
vecopmod(struct wpmopcode *op)
{
    struct vecopcode *vop = (struct vecopcode *)op;

    vecvmintop2(vop, vecmod);
    wpm->cpustat.pc += 4 + (vop->narg << 2);

    return;
}

void
vecoplt(struct wpmopcode *op)
{
    struct vecopcode *vop = (struct vecopcode *)op;

    vecvmop2(vop, veclt);
    wpm->cpustat.pc += 4 + (vop->narg << 2);

    return;
}

void
vecoplte(struct wpmopcode *op)
{
    struct vecopcode *vop = (struct vecopcode *)op;

    vecvmop2(vop, veclte);
    wpm->cpustat.pc += 4 + (vop->narg << 2);

    return;
}

void
vecopgt(struct wpmopcode *op)
{
    struct vecopcode *vop = (struct vecopcode *)op;

    vecvmop2(vop, vecgt);
    wpm->cpustat.pc += 4 + (vop->narg << 2);

    return;
}

void
vecopgte(struct wpmopcode *op)
{
    struct vecopcode *vop = (struct vecopcode *)op;

    vecvmop2(vop, vecgte);
    wpm->cpustat.pc += 4 + (vop->narg << 2);

    return;
}

void
vecopeq(struct wpmopcode *op)
{
    struct vecopcode *vop = (struct vecopcode *)op;

    vecvmop2(vop, veceq);
    wpm->cpustat.pc += 4 + (vop->narg << 2);

    return;
}

void
vecopineq(struct wpmopcode *op)
{
    struct vecopcode *vop = (struct vecopcode *)op;

    vecvmop2(vop, vecineq);
    wpm->cpustat.pc += 4 + (vop->narg << 2);

    return;
}

void
vecopshl(struct wpmopcode *op)
{
    struct vecopcode *vop = (struct vecopcode *)op;

    vecvmintop2(vop, vecshl);
    wpm->cpustat.pc += 4 + (vop->narg << 2);

    return;
}

void
vecopshr(struct wpmopcode *op)
{
    struct vecopcode *vop = (struct vecopcode *)op;

    vecvmintop2(vop, vecshr);
    wpm->cpustat.pc += 4 + (vop->narg << 2);

    return;
}

void
vecopnot(struct wpmopcode *op)
{
    struct vecopcode *vop = (struct vecopcode *)op;

    vecvmintop1(vop, vecnot);
    wpm->cpustat.pc += 4 + (vop->narg << 2);

    return;
}

void
vecopand(struct wpmopcode *op)
{
    struct vecopcode *vop = (struct vecopcode *)op;

    vecvmintop2(vop, vecand);
    wpm->cpustat.pc += 4 + (vop->narg << 2);

    return;
}

void
vecopor(struct wpmopcode *op)
{
    struct vecopcode *vop = (struct vecopcode *)op;

    vecvmintop2(vop, vecor);
    wpm->cpustat.pc += 4 + (vop->narg << 2);

    return;
}

void
vecopxor(struct wpmopcode *op)
{
    struct vecopcode *vop = (struct vecopcode *)op;

    vecvmintop2(vop, vecxor);
    wpm->cpustat.pc += 4 + (vop->narg << 2);

    return;
}

/* TODO: three arguments */
void
vecopselect(struct wpmopcode *op)
{
    struct vecopcode *vop = (struct vecopcode *)op;

//    vecvmop2(vop, vecselect);

    return;
}

void
vecoprand(struct wpmopcode *op)
{
    struct vecopcode *vop = (struct vecopcode *)op;

    vecvmintop1(vop, vecrand);
    wpm->cpustat.pc += 4 + (vop->narg << 2);

    return;
}

void
vecopfloor(struct wpmopcode *op)
{
    struct vecopcode *vop = (struct vecopcode *)op;

    vecvmfloatop1(vop, vecfloor);
    wpm->cpustat.pc += 4 + (vop->narg << 2);

    return;
}

void
vecopceil(struct wpmopcode *op)
{
    struct vecopcode *vop = (struct vecopcode *)op;

    vecvmfloatop1(vop, vecceil);
    wpm->cpustat.pc += 4 + (vop->narg << 2);

    return;
}

void
vecoptrunc(struct wpmopcode *op)
{
    struct vecopcode *vop = (struct vecopcode *)op;

    return;
}

void
vecopround(struct wpmopcode *op)
{
    struct vecopcode *vop = (struct vecopcode *)op;

    return;
}

void
vecopitof(struct wpmopcode *op)
{
    struct vecopcode *vop = (struct vecopcode *)op;

    return;
}

void
vecopitob(struct wpmopcode *op)
{
    struct vecopcode *vop = (struct vecopcode *)op;

    return;
}

void
vecopbtoi(struct wpmopcode *op)
{
    struct vecopcode *vop = (struct vecopcode *)op;

    return;
}

void
vecoplog(struct wpmopcode *op)
{
    struct vecopcode *vop = (struct vecopcode *)op;

    vecvmfloatop1(vop, veclog);
    wpm->cpustat.pc += 4 + (vop->narg << 2);

    return;
}

void
vecopsqrt(struct wpmopcode *op)
{
    struct vecopcode *vop = (struct vecopcode *)op;

    vecvmfloatop1(vop, vecsqrt);
    wpm->cpustat.pc += 4 + (vop->narg << 2);

    return;
}

void
vecopexp(struct wpmopcode *op)
{
    struct vecopcode *vop = (struct vecopcode *)op;

    return;
}

void
vecopsin(struct wpmopcode *op)
{
    struct vecopcode *vop = (struct vecopcode *)op;

    vecvmfloatop1(vop, vecsin);
    wpm->cpustat.pc += 4 + (vop->narg << 2);

    return;
}

void
vecopcos(struct wpmopcode *op)
{
    struct vecopcode *vop = (struct vecopcode *)op;

    vecvmfloatop1(vop, veccos);
    wpm->cpustat.pc += 4 + (vop->narg << 2);

    return;
}

void
vecoptan(struct wpmopcode *op)
{
    struct vecopcode *vop = (struct vecopcode *)op;

    vecvmfloatop1(vop, vectan);
    wpm->cpustat.pc += 4 + (vop->narg << 2);

    return;
}

void
vecopasin(struct wpmopcode *op)
{
    struct vecopcode *vop = (struct vecopcode *)op;

    vecvmfloatop1(vop, vecasin);
    wpm->cpustat.pc += 4 + (vop->narg << 2);

    return;
}

void
vecopacos(struct wpmopcode *op)
{
    struct vecopcode *vop = (struct vecopcode *)op;

    vecvmfloatop1(vop, vecacos);
    wpm->cpustat.pc += 4 + (vop->narg << 2);

    return;
}

void
vecopatan(struct wpmopcode *op)
{
    struct vecopcode *vop = (struct vecopcode *)op;

    vecvmfloatop1(vop, vecatan);
    wpm->cpustat.pc += 4 + (vop->narg << 2);

    return;
}

void
vecopsinh(struct wpmopcode *op)
{
    struct vecopcode *vop = (struct vecopcode *)op;

    vecvmfloatop1(vop, vecsinh);
    wpm->cpustat.pc += 4 + (vop->narg << 2);

    return;
}

void
vecopcosh(struct wpmopcode *op)
{
    struct vecopcode *vop = (struct vecopcode *)op;

    vecvmfloatop1(vop, veccosh);
    wpm->cpustat.pc += 4 + (vop->narg << 2);

    return;
}

void
vecoptanh(struct wpmopcode *op)
{
    struct vecopcode *vop = (struct vecopcode *)op;

    vecvmfloatop1(vop, vectanh);
    wpm->cpustat.pc += 4 + (vop->narg << 2);

    return;
}

#if (VECFULL)

void
vecopplscan(struct wpmopcode *op)
{
    struct vecopcode *vop = (struct vecopcode *)op;

    return;
}

void
vecopmulscan(struct wpmopcode *op)
{
    struct vecopcode *vop = (struct vecopcode *)op;

    return;
}

void
vecopmaxscan(struct wpmopcode *op)
{
    struct vecopcode *vop = (struct vecopcode *)op;

    return;
}

void
vecopminscan(struct wpmopcode *op)
{
    struct vecopcode *vop = (struct vecopcode *)op;

    return;
}

void
vecopandscan(struct wpmopcode *op)
{
    struct vecopcode *vop = (struct vecopcode *)op;

    return;
}

void
vecoporscan(struct wpmopcode *op)
{
    struct vecopcode *vop = (struct vecopcode *)op;

    return;
}

void
vecopxorscan(struct wpmopcode *op)
{
    struct vecopcode *vop = (struct vecopcode *)op;

    return;
}

void
vecopplreduce(struct wpmopcode *op)
{
    struct vecopcode *vop = (struct vecopcode *)op;

    return;
}

void
vecopmulreduce(struct wpmopcode *op)
{
    struct vecopcode *vop = (struct vecopcode *)op;

    return;
}

void
vecopmaxreduce(struct wpmopcode *op)
{
    struct vecopcode *vop = (struct vecopcode *)op;

    return;
}

void
vecopminreduce(struct wpmopcode *op)
{
    struct vecopcode *vop = (struct vecopcode *)op;

    return;
}
void
vecopandreduce(struct wpmopcode *op)

{
    struct vecopcode *vop = (struct vecopcode *)op;

    return;
}

void
vecoporreduce(struct wpmopcode *op)
{
    struct vecopcode *vop = (struct vecopcode *)op;

    return;
}

void
vecopxorreduce(struct wpmopcode *op)
{
    struct vecopcode *vop = (struct vecopcode *)op;

    return;
}

void
vecoppermute(struct wpmopcode *op)
{
    struct vecopcode *vop = (struct vecopcode *)op;

    return;
}

void
vecopdpermute(struct wpmopcode *op)
{
    struct vecopcode *vop = (struct vecopcode *)op;

    return;
}

void
vecopfpermute(struct wpmopcode *op)
{
    struct vecopcode *vop = (struct vecopcode *)op;

    return;
}

void
vecopbpermute(struct wpmopcode *op)
{
    struct vecopcode *vop = (struct vecopcode *)op;

    return;
}

void
vecopbfpermute(struct wpmopcode *op)
{
    struct vecopcode *vop = (struct vecopcode *)op;

    return;
}

void
vecopdfpermute(struct wpmopcode *op)
{
    struct vecopcode *vop = (struct vecopcode *)op;

    return;
}

void
vecopextract(struct wpmopcode *op)
{
    struct vecopcode *vop = (struct vecopcode *)op;

    return;
}

void
vecopreplace(struct wpmopcode *op)
{
    struct vecopcode *vop = (struct vecopcode *)op;

    return;
}

void
vecoppack(struct wpmopcode *op)
{
    struct vecopcode *vop = (struct vecopcode *)op;

    return;
}

void
vecoprankup(struct wpmopcode *op)
{
    struct vecopcode *vop = (struct vecopcode *)op;

    return;
}

void
vecoprankdown(struct wpmopcode *op)
{
    struct vecopcode *vop = (struct vecopcode *)op;

    return;
}

void
vecopdist(struct wpmopcode *op)
{
    struct vecopcode *vop = (struct vecopcode *)op;

    return;
}

void
vecopindex(struct wpmopcode *op)
{
    struct vecopcode *vop = (struct vecopcode *)op;

    return;
}

void
vecoplength(struct wpmopcode *op)
{
    struct vecopcode *vop = (struct vecopcode *)op;

    return;
}

void
vecopmkdes(struct wpmopcode *op)
{
    struct vecopcode *vop = (struct vecopcode *)op;

    return;
}

void
vecoplengths(struct wpmopcode *op)
{
    struct vecopcode *vop = (struct vecopcode *)op;

    return;
}

void
vecopcopy(struct wpmopcode *op)
{
    struct vecopcode *vop = (struct vecopcode *)op;

    return;
}

void
vecoppop(struct wpmopcode *op)
{
    struct vecopcode *vop = (struct vecopcode *)op;

    return;
}

void
vecopcpop(struct wpmopcode *op)
{
    struct vecopcode *vop = (struct vecopcode *)op;

    return;
}

void
vecoppair(struct wpmopcode *op)
{
    struct vecopcode *vop = (struct vecopcode *)op;

    return;
}

void
vecopunpair(struct wpmopcode *op)
{
    struct vecopcode *vop = (struct vecopcode *)op;

    return;
}

#endif /* WPMVECFULL */

#endif /* WPMVEC */

