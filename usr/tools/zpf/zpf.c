#include <limits.h>
#include <zero/cdefs.h>
#include <zero/param.h>
#include <zpf/zpf.h>

/* initialise dispatch function pointer tables */
void
zpfinitopfuncs(struct zpfopinfo *info)
{
    zpfopfunc *tab;

    info->unitfuncs[ZPF_ALU_UNIT] = info->alufunctab;
    info->unitfuncs[ZPF_MEM_UNIT] = info->memfunctab;
    info->unitfuncs[ZPF_FLOW_UNIT] = info->flowfunctab;
    info->unitfuncs[ZPF_XFER_UNIT] = info->xferfunctab;

    tab = info->alufunctab;
    tab[ZPF_ADD] = zpfaddfunc;
    tab[ZPF_SUB] = zpfsubfunc;
    tab[ZPF_MUL] = zpfmulfunc;
    tab[ZPF_DIV] = zpfdivfunc;
    tab[ZPF_MOD] = zpfmodfunc;
    tab[ZPF_NEG] = zpfnegfunc;
    tab[ZPF_AND] = zpfandfunc;
    tab[ZPF_OR] = zpforfunc;
    tab[ZPF_XOR] = zpfxorfunc;
    tab[ZPF_LSH] = zpflshfunc;
    tab[ZPF_RSH] = zpfrshfunc;

    tab = info->memfunctab;
    tab[ZPF_LD] = zpfldfunc;
    tab[ZPF_LDI] = zpfldifunc;
    tab[ZPF_LDH] = zpfldhfunc;
    tab[ZPF_LDB] = zpfldbfunc;
    tab[ZPF_LDX] = zpfldxfunc;
    tab[ZPF_LDXI] = zpfldxifunc;
    tab[ZPF_LDXB] = zpfldxbfunc;
    tab[ZPF_ST] = zpfstfunc;
    tab[ZPF_STX] = zpfstxfunc;
    
    tab = info->flowfunctab;
    tab[ZPF_JMP] = zpfjmpfunc;
    tab[ZPF_JA] = zpfjafunc;
    tab[ZPF_JEQ] = zpfjeqfunc;
    tab[ZPF_JNE] = zpfjnefunc;
    tab[ZPF_JNEQ] = zpfjnefunc;
    tab[ZPF_JLT] = zpfjltfunc;
    tab[ZPF_JLE] = zpjlefunc;
    tab[ZPF_JGT] = zpfjgtfunc;
    tab[ZPF_JGE] = zpfjgefunc;
    tab[ZPF_JSET] = zpfjsetfunc;
    tab[ZPF_RET] = zpfretfunc;

    tab = info->xferfunctab;
    tab[ZPF_TAX] = zpftaxfunc;
    tab[ZPF_TXA] = zpftxafunc;

    return;
}

/* initialise instruction argument counts */
void
zpfinitopnargs(struct zpfopinfo *info)
{
    zpfopfunc *tab;

    info->unitopnargs[ZPF_ALU_UNIT] = info->alunargtab;
    info->unitopnargs[ZPF_MEM_UNIT] = info->memnargtab;
    info->unitopnargs[ZPF_FLOW_UNIT] = info->flownargtab;
    info->unitopnargs[ZPF_XFER_UNIT] = info->xfernargtab;

    tab = info->alufunctab;
    tab[ZPF_ADD] = 2;
    tab[ZPF_SUB] = 2;
    tab[ZPF_MUL] = 2;
    tab[ZPF_DIV] = 2;
    tab[ZPF_MOD] = 2;
    tab[ZPF_AND] = 2;
    tab[ZPF_OR] = 2;
    tab[ZPF_XOR] = 2;
    tab[ZPF_LSH] = 2;
    tab[ZPF_RSH] = 2;
    tab[ZPF_NEG] = 1;

    tab = info->memfunctab;
    tab[ZPF_LD] = 2;
    tab[ZPF_LDI] = 2;
    tab[ZPF_LDH] = 2;
    tab[ZPF_LDB] = 2;
    tab[ZPF_LDX] = 2;
    tab[ZPF_LDXI] = 2;
    tab[ZPF_LDXB] = 2;
    tab[ZPF_ST] = 1;
    tab[ZPF_STX] = 1;
    
    tab = info->flowfunctab;
    tab[ZPF_JMP] = 1;
    tab[ZPF_JA] = 1;
    tab[ZPF_JEQ] = 2;
    tab[ZPF_JNE] = 2;
    tab[ZPF_JNEQ] = 2;
    tab[ZPF_JLT] = 2;
    tab[ZPF_JLE] = 2;
    tab[ZPF_JGT] = 2;
    tab[ZPF_JGE] = 2;
    tab[ZPF_JSET] = 2;
    tab[ZPF_RET] = 1;

    tab = info->xferfunctab;
    tab[ZPF_TAX] = 0;
    tab[ZPF_TXA] = 0;

    return;
}

/* init operation addressing mode bitmaps */
void
zpfinitopadrbits(struct zpfopinfo *info)
{
    uint16_t *tab;

    info->unitadrmodes[ZPF_ALU_UNIT] = zpf->aluadrbits;
    info->unitadrmodes[ZPF_MEM_UNIT] = zpf->memadrbits;
    info->unitadrmodes[ZPF_FLOW_UNIT] = zpf->flowadrbits;
    info->unitadrmodes[ZPF_XFER_UNIT] = zpf->xferadrbits;

    tab = zpf->aluadrbits;
    tab[ZPF_ADD] = ZPF_REG_ARG | ZPF_IMMED_ARG;
    tab[ZPF_SUB] = ZPF_REG_ARG | ZPF_IMMED_ARG;
    tab[ZPF_MUL] = ZPF_REG_ARG | ZPF_IMMED_ARG;
    tab[ZPF_DIV] = ZPF_REG_ARG | ZPF_IMMED_ARG;
    tab[ZPF_MOD] = ZPF_REG_ARG | ZPF_IMMED_ARG;
    tab[ZPF_NEG] = ZPF_REG_ARG | ZPF_IMMED_ARG;
    tab[ZPF_AND] = ZPF_REG_ARG | ZPF_IMMED_ARG;
    tab[ZPF_OR] = ZPF_REG_ARG | ZPF_IMMED_ARG;
    tab[ZPF_XOR] = ZPF_REG_ARG | ZPF_IMMED_ARG;
    tab[ZPF_LSH] = ZPF_REG_ARG | ZPF_IMMED_ARG;
    tab[ZPF_RSH] = ZPF_REG_ARG | ZPF_IMMED_ARG;

    tab = info->memfunctab;
    tab[ZPF_LD] = (ZPF_IMMED_ARG | ZPF_MEM_ARG
                   | ZPF_PKT_ARG | ZPF_PKT_OFS_BIT | ZPF_EXT_ARG);
    tab[ZPF_LDI] = ZPF_IMMED_ARG;
    tab[ZPF_LDH] = ZPF_PKT_ARG | ZPF_PKT_OFS_BIT;
    tab[ZPF_LDB] = ZPF_PKT_ARG | ZPF_PKT_OFS_BIT; 
    tab[ZPF_LDX] = (ZPF_IMMED_ARG | ZPF_MEM_ARG
                    | ZPF_PKT_ARG | ZPF_ADR_MUL4_BIT | ZPF_EXT_ARG);
    tab[ZPF_LDXI] = ZPF_IMMED_ARG;
    tab[ZPF_LDXB] = ZPF_PKT_ARG | ZPF_ADR_MUL4_BIT;
    tab[ZPF_ST] = ZPF_ADR_NONE;
    tab[ZPF_STX] = ZPF_ADR_NONE;
    
    tab = info->flowfunctab;
    tab[ZPF_JMP] = ZPF_SYM_ARG;
    tab[ZPF_JA] = ZPF_SYM_ARG;
    tab[ZPF_JEQ] = ZPF_JMP_ARG | ZPF_JMP_ARG2_BIT;
    tab[ZPF_JNE] = ZPF_JMP_ARG | ZPF_ADR_TRUE_BIT;
    tab[ZPF_JNEQ] = ZPF_JMP_ARG | ZPF_ADR_TRUE_BIT;
    tab[ZPF_JLT] = ZPF_JMP_ARG | ZPF_ADR_TRUE_BIT;
    tab[ZPF_JLE] = ZPF_JMP_ARG | ZPF_ADR_TRUE_BIT;
    tab[ZPF_JGT] = ZPF_JMP_ARG | ZPF_JMP_ARG2_BIT;
    tab[ZPF_JGE] = ZPF_JMP_ARG | ZPF_JMP_ARG2_BIT;
    tab[ZPF_JSET] = ZPF_JMP_ARG | ZPF_JMP_ARG2_BIT;
    tab[ZPF_RET] = zpfretfunc;

    tab = info->xferfunctab;
    tab[ZPF_TAX] = ZPF_ADR_NONE;
    tab[ZPF_TXA] = ZPF_ADR_NONE;

    return;
}

void
zpfinitopinfo(struct zpfopinfo *info)
{
    zpfinitopfuncs(info);
    zpfinitnargs(info);
    zpfinitadrbits(info);
}

