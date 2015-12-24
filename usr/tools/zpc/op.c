#include <zpc/conf.h>
#include <stdint.h>
#include <zpc/op.h>

void
zpcopnot(struct zpc *zpc, struct zpcopcode *op)
{
    zpcreg_t sreg = op->src;
    zpcreg_t dreg = op->dest;
    zpcreg_t sptr = &zpc->regs[sreg];
    zpcreg_t dptr = &zpc->regs[dreg];
    zpcreg_t pc = zcp->mregs[ZPCMREGPC];
    zpcreg_t dest = *dptr;

    zpc->mregs[ZPCMREGSW] &= ~(ZPCMSWZF | ZPCXMSWOF | ZPCMSWCF);
    dest = ~dest;
    pc += (1 + (zpc->flg & ZPCOPARGBIT)) * ZPCOPSIZE;
    zpcsetzf(zpc, dest);
    *dptr = dest;
    zpc->mregs[ZPCMREGPC] = pc;

    return;
}

void
zpcopand(struct zpc *zpc, struct zpcopcode *op)
{
    zpcreg_t sreg = op->src;
    zpcreg_t dreg = op->dest;
    zpcreg_t sptr = &zpc->regs[sreg];
    zpcreg_t dptr = &zpc->regs[dreg];
    zpcreg_t pc = zcp->mregs[ZPCMREGPC];
    zpcreg_t src = *sptr;
    zpcreg_t dest = *dptr;

    zpc->mregs[ZPCMREGSW] &= ~(ZPCMSWZF | ZPCXMSWOF | ZPCMSWCF);
    dest &= src;
    pc += (1 + (zpc->flg & ZPCOPARGBIT)) * ZPCOPSIZE;
    zpcsetzf(zpc, dest);
    *dptr = dest;
    zpc->mregs[ZPCMREGPC] = pc;

    return;
}

void
zpcopor(struct zpc *zpc, struct zpcopcode *op)
{
    zpcreg_t sreg = op->src;
    zpcreg_t dreg = op->dest;
    zpcreg_t sptr = &zpc->regs[sreg];
    zpcreg_t dptr = &zpc->regs[dreg];
    zpcreg_t pc = zcp->mregs[ZPCMREGPC];
    zpcreg_t src = *sptr;
    zpcreg_t dest = *dptr;

    zpc->mregs[ZPCMREGSW] &= ~(ZPCMSWZF | ZPCXMSWOF | ZPCMSWCF);
    dest |= src;
    pc += (1 + (zpc->flg & ZPCOPARGBIT)) * ZPCOPSIZE;
    zpcsetzf(zpc, dest);
    *dptr = dest;
    zpc->mregs[ZPCMREGPC] = pc;

    return;
}

void
zpcopxor(struct zpc *zpc, struct zpcopcode *op)
{
    zpcreg_t sreg = op->src;
    zpcreg_t dreg = op->dest;
    zpcreg_t sptr = &zpc->regs[sreg];
    zpcreg_t dptr = &zpc->regs[dreg];
    zpcreg_t pc = zcp->mregs[ZPCMREGPC];
    zpcreg_t src = *sptr;
    zpcreg_t dest = *dptr;

    zpc->mregs[ZPCMREGSW] &= ~(ZPCMSWZF | ZPCXMSWOF | ZPCMSWCF);
    dest ^= src;
    pc += (1 + (zpc->flg & ZPCOPARGBIT)) * ZPCOPSIZE;
    zpcsetzf(zpc, dest);
    *dptr = dest;
    zpc->mregs[ZPCMREGPC] = pc;

    return;
}

void
zpcopshl(struct zvmopcode *op)
{
    zpcreg_t  sreg = op->src;
    zpcreg_t  dreg = op->dest;
    zpcreg_t *sptr = &zpc->regs[sreg];
    zpcreg_t *dptr = &zpc->regs[dreg];
    zpcreg_t  pc = zcp->mregs[ZPCMREGPC];
    zpcreg_t  src = *sptr;
    zpcreg_t  dest = *dptr;

    zpc->mregs[ZPCMREGSW] &= ~(ZPCMSWZF | ZPCMSWOF | ZPCMSWCF);
    dest <<= src;
    pc += (1 + (zpc->flg & ZPCOPARGBIT)) * ZPCOPSIZE;
    zvmsetzf(dest);
    *dptr = dest;
    zpc->mregs[ZPCMREGPC] = pc;

    return;
}

void
zpcopshr(struct zvmopcode *op)
{
    zpcreg_t  sreg = op->src;
    zpcreg_t  dreg = op->dest;
    zpcreg_t *sptr = &zpc->regs[sreg];
    zpcreg_t *dptr = &zpc->regs[dreg];
    zpcreg_t  pc = zcp->mregs[ZPCMREGPC];
    zpcreg_t  src = *sptr;
    zpcreg_t  dest = *dptr;
    zpcreg_t  mask = ~((zpcreg_t)0) >> src;

    zpc->mregs[ZPCMREGSW] &= ~(ZPCMSWZF | ZPCMSWOF | ZPCMSWCF);
    dest >>= src;
    dest &= mask;
    pc += (1 + (zpc->flg & ZPCOPARGBIT)) * ZPCOPSIZE;
    zvmsetzf(dest);
    *dptr = dest;
    zpc->mregs[ZPCMREGPC] = pc;

    return;
}

void
zpcopsar(struct zvmopcode *op)
{
    zpcreg_t  sreg = op->src;
    zpcreg_t  dreg = op->dest;
    zpcreg_t *sptr = &zpc->regs[sreg];
    zpcreg_t *dptr = &zpc->regs[dreg];
    zpcreg_t  pc = zcp->mregs[ZPCMREGPC];
    zpcreg_t  src = *sptr;
    zpcreg_t  dest = *dptr;
    zpcreg_t  mask = ~(zpcreg_t)0;
#if (ZVM32BIT)
    zpcreg_t  sign = (((dest) & (1 << 31))
                      ? (mask >> (32 - src))
                      : 0);
#else
    zpcreg_t  sign = (((dest) & (INT64_C(1) << 63))
                      ? (mask >> (64 - src))
                      : 0);
#endif

    zpc->mregs[ZPCMREGSW] &= ~(ZPCMSWZF | ZPCMSWOF | ZPCMSWCF);
    dest <<= src;
    pc += (1 + (zpc->flg & ZPCOPARGBIT)) * ZPCOPSIZE;
    dest |= sign;
    zvmsetzf(dest);
    *dptr = dest;
    zpc->mregs[ZPCMREGPC] = pc;

    return;
}

