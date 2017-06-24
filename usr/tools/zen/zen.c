#include <stdint.h>
#include <limits.h>
#include <zero/cdefs.h>
#include <zero/param.h>
#include <zero/trix.h>
#include <zen/zen.h>

void
zeninitopspacetab(struct zenopinfo *info)
{
    memset(info->nospacemap, 0, 256 * sizeof(unsigned char) / CHAR_BIT);
    setbit(info->nospacetab, '%');
    setbit(info->nospacetab, '#');
}

/* initialise dispatch function pointer tables */
void
zeninitopfuncs(struct zenopinfo *info)
{
    zenopfunc *tab;

    info->unitfuncs[ZEN_ALU_UNIT] = info->alufunctab;
    info->unitfuncs[ZEN_MEM_UNIT] = info->memfunctab;
    info->unitfuncs[ZEN_FLOW_UNIT] = info->flowfunctab;
    info->unitfuncs[ZEN_XFER_UNIT] = info->xferfunctab;

    tab = info->alufunctab;
    tab[ZEN_ADD] = zenaddfunc;
    tab[ZEN_SUB] = zensubfunc;
    tab[ZEN_MUL] = zenmulfunc;
    tab[ZEN_DIV] = zendivfunc;
    tab[ZEN_MOD] = zenmodfunc;
    tab[ZEN_NEG] = zennegfunc;
    tab[ZEN_AND] = zenandfunc;
    tab[ZEN_OR] = zenorfunc;
    tab[ZEN_XOR] = zenxorfunc;
    tab[ZEN_LSH] = zenlshfunc;
    tab[ZEN_RSH] = zenrshfunc;

    tab = info->memfunctab;
    tab[ZEN_LD] = zenldfunc;
    tab[ZEN_LDI] = zenldifunc;
    tab[ZEN_LDH] = zenldhfunc;
    tab[ZEN_LDB] = zenldbfunc;
    tab[ZEN_LDX] = zenldxfunc;
    tab[ZEN_LDXI] = zenldxifunc;
    tab[ZEN_LDXB] = zenldxbfunc;
    tab[ZEN_ST] = zenstfunc;
    tab[ZEN_STX] = zenstxfunc;
    
    tab = info->flowfunctab;
    tab[ZEN_JMP] = zenjmpfunc;
    tab[ZEN_JA] = zenjafunc;
    tab[ZEN_JEQ] = zenjeqfunc;
    tab[ZEN_JNE] = zenjnefunc;
    tab[ZEN_JNEQ] = zenjnefunc;
    tab[ZEN_JLT] = zenjltfunc;
    tab[ZEN_JLE] = zpjlefunc;
    tab[ZEN_JGT] = zenjgtfunc;
    tab[ZEN_JGE] = zenjgefunc;
    tab[ZEN_JSET] = zenjsetfunc;
    tab[ZEN_RET] = zenretfunc;

    tab = info->xferfunctab;
    tab[ZEN_TAX] = zentaxfunc;
    tab[ZEN_TXA] = zentxafunc;

    return;
}

/* initialise instruction argument counts */
void
zeninitopnargs(struct zenopinfo *info)
{
    zenopfunc *tab;

    info->unitopnargs[ZEN_ALU_UNIT] = info->alunargtab;
    info->unitopnargs[ZEN_MEM_UNIT] = info->memnargtab;
    info->unitopnargs[ZEN_FLOW_UNIT] = info->flownargtab;
    info->unitopnargs[ZEN_XFER_UNIT] = info->xfernargtab;

    tab = info->alufunctab;
    tab[ZEN_ADD] = 2;
    tab[ZEN_SUB] = 2;
    tab[ZEN_MUL] = 2;
    tab[ZEN_DIV] = 2;
    tab[ZEN_MOD] = 2;
    tab[ZEN_AND] = 2;
    tab[ZEN_OR] = 2;
    tab[ZEN_XOR] = 2;
    tab[ZEN_LSH] = 2;
    tab[ZEN_RSH] = 2;
    tab[ZEN_NEG] = 1;

    tab = info->memfunctab;
    tab[ZEN_LD] = 2;
    tab[ZEN_LDI] = 2;
    tab[ZEN_LDH] = 2;
    tab[ZEN_LDB] = 2;
    tab[ZEN_LDX] = 2;
    tab[ZEN_LDXI] = 2;
    tab[ZEN_LDXB] = 2;
    tab[ZEN_ST] = 1;
    tab[ZEN_STX] = 1;
    
    tab = info->flowfunctab;
    tab[ZEN_JMP] = 1;
    tab[ZEN_JA] = 1;
    tab[ZEN_JEQ] = 2;
    tab[ZEN_JNE] = 2;
    tab[ZEN_JNEQ] = 2;
    tab[ZEN_JLT] = 2;
    tab[ZEN_JLE] = 2;
    tab[ZEN_JGT] = 2;
    tab[ZEN_JGE] = 2;
    tab[ZEN_JSET] = 2;
    tab[ZEN_RET] = 1;

    tab = info->xferfunctab;
    tab[ZEN_TAX] = 0;
    tab[ZEN_TXA] = 0;

    return;
}

/* init operation addressing mode bitmaps */
void
zeninitopadrbits(struct zenopinfo *info)
{
    uint16_t *tab;

    info->unitadrmodes[ZEN_ALU_UNIT] = zen->aluadrbits;
    info->unitadrmodes[ZEN_MEM_UNIT] = zen->memadrbits;
    info->unitadrmodes[ZEN_FLOW_UNIT] = zen->flowadrbits;
    info->unitadrmodes[ZEN_XFER_UNIT] = zen->xferadrbits;

    tab = zen->aluadrbits;
    tab[ZEN_ADD] = ZEN_REG_ARG | ZEN_IMMED_ARG;
    tab[ZEN_SUB] = ZEN_REG_ARG | ZEN_IMMED_ARG;
    tab[ZEN_MUL] = ZEN_REG_ARG | ZEN_IMMED_ARG;
    tab[ZEN_DIV] = ZEN_REG_ARG | ZEN_IMMED_ARG;
    tab[ZEN_MOD] = ZEN_REG_ARG | ZEN_IMMED_ARG;
    tab[ZEN_NEG] = ZEN_REG_ARG | ZEN_IMMED_ARG;
    tab[ZEN_AND] = ZEN_REG_ARG | ZEN_IMMED_ARG;
    tab[ZEN_OR] = ZEN_REG_ARG | ZEN_IMMED_ARG;
    tab[ZEN_XOR] = ZEN_REG_ARG | ZEN_IMMED_ARG;
    tab[ZEN_LSH] = ZEN_REG_ARG | ZEN_IMMED_ARG;
    tab[ZEN_RSH] = ZEN_REG_ARG | ZEN_IMMED_ARG;

    tab = info->memfunctab;
    tab[ZEN_LD] = (ZEN_IMMED_ARG | ZEN_MEM_ARG
                   | ZEN_PKT_ARG | ZEN_PKT_OFS_BIT | ZEN_EXT_ARG);
    tab[ZEN_LDI] = ZEN_IMMED_ARG;
    tab[ZEN_LDH] = ZEN_PKT_ARG | ZEN_PKT_OFS_BIT;
    tab[ZEN_LDB] = ZEN_PKT_ARG | ZEN_PKT_OFS_BIT; 
    tab[ZEN_LDX] = (ZEN_IMMED_ARG | ZEN_MEM_ARG
                    | ZEN_PKT_ARG | ZEN_ADR_MUL4_BIT | ZEN_EXT_ARG);
    tab[ZEN_LDXI] = ZEN_IMMED_ARG;
    tab[ZEN_LDXB] = ZEN_PKT_ARG | ZEN_ADR_MUL4_BIT;
    tab[ZEN_ST] = ZEN_ADR_NONE;
    tab[ZEN_STX] = ZEN_ADR_NONE;
    
    tab = info->flowfunctab;
    tab[ZEN_JMP] = ZEN_SYM_ARG;
    tab[ZEN_JA] = ZEN_SYM_ARG;
    tab[ZEN_JEQ] = ZEN_JMP_ARG | ZEN_JMP_ARG2_BIT;
    tab[ZEN_JNE] = ZEN_JMP_ARG | ZEN_ADR_TRUE_BIT;
    tab[ZEN_JNEQ] = ZEN_JMP_ARG | ZEN_ADR_TRUE_BIT;
    tab[ZEN_JLT] = ZEN_JMP_ARG | ZEN_ADR_TRUE_BIT;
    tab[ZEN_JLE] = ZEN_JMP_ARG | ZEN_ADR_TRUE_BIT;
    tab[ZEN_JGT] = ZEN_JMP_ARG | ZEN_JMP_ARG2_BIT;
    tab[ZEN_JGE] = ZEN_JMP_ARG | ZEN_JMP_ARG2_BIT;
    tab[ZEN_JSET] = ZEN_JMP_ARG | ZEN_JMP_ARG2_BIT;
    tab[ZEN_RET] = zenretfunc;

    tab = info->xferfunctab;
    tab[ZEN_TAX] = ZEN_ADR_NONE;
    tab[ZEN_TXA] = ZEN_ADR_NONE;

    return;
}

void
zeninitopinfo(struct zenopinfo *info)
{
    zeninitnospacetab(info);
    zeninitopfuncs(info);
    zeninitnargs(info);
    zeninitadrbits(info);
}

