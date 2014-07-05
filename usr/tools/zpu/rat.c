#include <zpu/conf.h>

#if (ZPURAT)

#include <zero/trix.h>
#include <zpu/zpu.h>
#include <zpu/rat.h>

/* Thanks to Jeremy 'jercos' Sturdivant for radd() and rsub(). */

void
radd(struct zpu *zpu, struct zpuop *op)
{
    int64_t sreg = op->src;
    int64_t dreg = op->dest;
    int64_t src = zpu->ctx.regs[sreg];
    int64_t dest = zpu->ctx.regs[dreg];
    int64_t snum = zpugetnum64(src);
    int64_t sden = zpugetdenom64(src);
    int64_t dnum = zpugetnum64(dest);
    int64_t dden = zpugetdenom64(dest);

    dnum *= sden;
    dnum += snum * dden;
    dden *= sden;
    if ((dnum & INT64_C(0xffffffff00000000))
        || (dden & INT64_C(0xffffffff00000000))) {
        zpu->ctx.regs[ZPUMSWREG] |= MSW_VF;
    }
    ratreduce(&dnum, &dden);
    zpusetnum64(dest, dnum);
    zpusetdenom64(dest, dden);
    zpu->ctx.regs[dreg] = dest;
    zpu->ctx.regs[ZPUPCREG] += 4;
}

void
rsub(struct zpu *zpu, struct zpuop *op)
{
    int64_t sreg = op->src;
    int64_t dreg = op->dest;
    int64_t src = zpu->ctx.regs[sreg];
    int64_t dest = zpu->ctx.regs[dreg];
    int64_t snum = zpugetnum64(src);
    int64_t sden = zpugetdenom64(src);
    int64_t dnum = zpugetnum64(dest);
    int64_t dden = zpugetdenom64(dest);

    dnum = snum * dden - sden * dnum;
    dden *= sden;
    if ((dnum & INT64_C(0xffffffff00000000))
        || (dden & INT64_C(0xffffffff00000000))) {
        zpu->ctx.regs[ZPUMSWREG] |= MSW_VF;
    }
    ratreduce(&dnum, &dden);
    zpusetnum64(dest, dnum);
    zpusetdenom64(dest, dden);
    zpu->ctx.regs[dreg] = dest;
    zpu->ctx.regs[ZPUPCREG] += 4;
}

void
rmul(struct zpu *zpu, struct zpuop *op)
{
    int64_t sreg = op->src;
    int64_t dreg = op->dest;
    int64_t src = zpu->ctx.regs[sreg];
    int64_t dest = zpu->ctx.regs[dreg];
    int64_t snum = zpugetnum64(src);
    int64_t sden = zpugetdenom64(src);
    int64_t dnum = zpugetnum64(dest);
    int64_t dden = zpugetdenom64(dest);

    dnum *= snum;
    dden *= sden;
    if ((dnum & INT64_C(0xffffffff00000000))
        || (dden & INT64_C(0xffffffff00000000))) {
        zpu->ctx.regs[ZPUMSWREG] |= MSW_VF;
    }
    zpusetnum64(dest, dnum);
    zpusetdenom64(dest, dden);
    zpu->ctx.regs[dreg] = dest;
    zpu->ctx.regs[ZPUPCREG] += 4;
}

void
rdiv(struct zpu *zpu, struct zpuop *op)
{
    int64_t sreg = op->src;
    int64_t dreg = op->dest;
    int64_t src = zpu->ctx.regs[sreg];
    int64_t dest = zpu->ctx.regs[dreg];
    int64_t snum = zpugetnum64(src);
    int64_t sden = zpugetdenom64(src);
    int64_t dnum = zpugetnum64(dest);
    int64_t dden = zpugetdenom64(dest);

    dnum *= sden;
    dden *= snum;
    if ((dnum & INT64_C(0xffffffff00000000))
        || (dden & INT64_C(0xffffffff00000000))) {
        zpu->ctx.regs[ZPUMSWREG] |= MSW_VF;
    }
    zpusetnum64(dest, dnum);
    zpusetdenom64(dest, dden);
    zpu->ctx.regs[dreg] = dest;
    zpu->ctx.regs[ZPUPCREG] += 4;
}

#endif /* ZPURAT */

