#include <zpu/conf.h>
#if (ZPURAT)

#include <zpu/zpu.h>
#include <zpu/rat.h>

void
radd(struct zpu *zpu, struct zpuop *op)
{
    ;
}

void
rsub(struct zpu *zpu, struct zpuop *op)
{
    ;
}

void
rmul(struct zpu *zpu, struct zpuop *op)
{
    int64_t sreg = op->src;
    int64_t dreg = op->dest;
    int64_t src = zpu->ctx.regs[sreg];
    int64_t dest = zpu->ctx.regs[dreg];
    int64_t snom = zpugetnom64(src);
    int64_t sden = zpugetdenom64(src);
    int64_t dnom = zpugetnom64(dest);
    int64_t dden = zpugetdenom64(dest);

    dnom *= snom;
    dden *= sden;
    zpusetnom64(dest, dnom);
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
    int64_t snom = zpugetnom64(src);
    int64_t sden = zpugetdenom64(src);
    int64_t dnom = zpugetnom64(dest);
    int64_t dden = zpugetdenom64(dest);

    dnom *= sden;
    dden *= snom;
    zpusetnom64(dest, dnom);
    zpusetdenom64(dest, dden);
    zpu->ctx.regs[dreg] = dest;
    zpu->ctx.regs[ZPUPCREG] += 4;
}

#endif /* ZPURAT */

