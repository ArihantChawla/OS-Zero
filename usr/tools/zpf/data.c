#include <string.h>

struct zpmsym  *zpmsymhash[ZPFNHASH];
struct zpminst *zpminsthash[ZPFNHASH];
struct zpminst  zpminsttab[ZPM_NUNIT][ZPM_UNIT_NOP];
const char     *zpminstnametab[ZPM_NUNIT][ZPM_UNIT_NOP];
const char     *zpminstnametab[ZPM_NUNIT][ZPM_UNIT_NOP];
uint32_t        zpfinstargbits[ZPF_NUNIT][ZPF_UNIT_NOP];
unsigned char   zpfreqargsmap[256];
unsigned char   zpfnospacemap[256 / CHAR_BIT];
zpfopfunc      *zpfopfunctab[ZPF_OP_NUNIT][ZPF_OP_NINST];
zpfop         **zpfoptab;


void
zpminitinstnames(void)
{
    long            unit;
    
    unit = ZPM_ALU_UNIT;
    zpminstnametab[unit][ZPM_ADD] = strdup("add");
    zpminstnametab[unit][ZPF_SUB] = strdup("sub");
    zpminstnametab[unit][ZPF_MUL] = strdup("mul");
    zpminstnametab[unit][ZPF_DIV] = strdup("div");
    zpminstnametab[unit][ZPF_MOD] = strdup("mod");
    zpminstnametab[unit][ZPF_AND = strdup("and");
    zpminstnametab[unit][ZPF_OR] = strdup("or"); 
    zpminstnametab[unit][ZPF_XOR] = strdup("xor");
    zpminstnametab[unit][ZPF_LSH] = strdup("lsh");
    zpminstnametab[unit][ZPF_RSH] = strdup("rsh");
    zpminstnametab[unit][ZPF_NEG] = strdup("neg");
    unit = ZPF_REG_UNIT;
    zpminstnametab[unit][ZPF_LDB] = strdup("ldb");
    zpminstnametab[unit][ZPF_LDH] = strdup("ldh");
    zpminstnametab[unit][ZPF_LDI] = strdup("ldi");
    zpminstnametab[unit][ZPF_LD] = strdup("ld");
    zpminstnametab[unit][ZPF_LDX] = strdup("ldx");
    zpminstnametab[unit][ZPF_LDXI] = strdup("ldxi");
    zpminstnametab[unit][ZPF_LDXB] = strdup("ldxb");
    zpminstnametab[unit][ZPF_ST] = strdup("st");
    zpminstnametab[unit][ZPF_STX] = strdup("stx");
    unit = ZPM_FLOW_UNIT;
    zpminstnametab[unit][ZPF_JMP] = strdup("jmp");
    zpminstnametab[unit][ZPF_JA] = strdup("ja");
    zpminstnametab[unit][ZPF_JEQ] = strdup("jeq");
    zpminstnametab[unit][ZPF_JNE] = strdup("jne");
    zpminstnametab[unit][ZPF_JLT] = strdup("jlt");
    zpminstnametab[unit][ZPF_JLE] = strdup("jle");
    zpminstnametab[unit][ZPF_JGT] = strdup("jgt");
    zpminstnametab[unit][ZPF_JGE] = strdup("jge");
    zpminstnametab[unit][ZPF_JSET] = strdup("jset");
    zpminstnametab[unit][ZPF_RET] = strdup("ret");
    unit = ZPM_XFER_UNIT;
    zpminstnametab[unit][ZPF_TAX] = strdup("tax");
    zpminstnametab[unit][ZPF_TXA] = strdup("txa");
    
    return;
}
        
void
zpfinitinstargbits(void)
{
    sbits = ZPF_X_BIT | ZPF_K_BIT;
    dbits = ZPF_A_BIT;
    zpfinstargbits[ZPF_ALU_UNIT][ZPF_ADD] = zpfmergeargbits(sbits, dbits);
    zpfinstargbits[ZPF_ALU_UNIT][ZPF_SUB] = zpfmergeargbits(sbits, dbits);
    zpfinstargbits[ZPF_ALU_UNIT][ZPF_MUL] = zpfmergeargbits(sbits, dbits);
    zpfinstargbits[ZPF_ALU_UNIT][ZPF_DIV] = zpfmergeargbits(sbits, dbits);
    zpfinstargbits[ZPF_ALU_UNIT][ZPF_MOD] = zpfmergeargbits(sbits, dbits);
    zpfinstargbits[ZPF_ALU_UNIT][ZPF_AND] = zpfmergeargbits(sbits, dbits);
    zpfinstargbits[ZPF_ALU_UNIT][ZPF_OR] = zpfmergeargbits(sbits, dbits);
    zpfinstargbits[ZPF_ALU_UNIT][ZPF_XOR] = zpfmergeargbits(sbits, dbits);
    zpfinstargbits[ZPF_ALU_UNIT][ZPF_LSH] = zpfmergeargbits(sbits, dbits);
    zpfinstargbits[ZPF_ALU_UNIT][ZPF_RSH] = zpfmergeargbits(sbits, dbits);
    sbits = ZPF_A_BIT;
    zpfinstargbits[ZPF_ALU_UNIT][ZPF_NEG] = zpfmergeargbits(sbits, dbits);
    sbits = ZPF_K_BIT | ZPF_OFS_BIT;
    dbits = ZPF_A_BIT;
    zpfinstargbits[ZPF_REG_UNIT][ZPF_LDB] = zpfmergeargbits(sbits, dbits);
    zpfinstargbits[ZPF_REG_UNIT][ZPF_LDH] = zpfmergeargbits(sbits, dbits);
    sbits = ZPF_K_BIT;
    dbits = ZPF_A_BIT;
    zpfinstargbits[ZPF_REG_UNIT][ZPF_LDI] = zpfmergeargbits(sbits, dbits);
    sbits = ZPF_K_BIT | ZPF_MEM_BIT | ZPF_PKT_BIT | ZPF_OFS_BIT;
    dbits = ZPF_A_BIT;
    zpfinstargbits[ZPF_REG_UNIT][ZPF_LD] = zpfmergeargbits(sbits, dbits);
    dbits = ZPF_X_BIT;
    zpfinstargbits[ZPF_REG_UNIT][ZPF_LX] = zpfmergeargbits(sbits, dbits);
    sbits = ZPF_K_BIT;
    zpfinstargbits[ZPF_REG_UNIT][ZPF_LDXI] = zpfmergeargbits(sbits, dbits);
    sbits = ZPF_PKT_BIT | ZPF_MUL4_BIT;
    zpfinstargbits[ZPF_REG_UNIT][ZPF_LDXB] = zpfmergeargbits(sbits, dbits);
    sbits = ZPF_A_BIT;
    dbits = ZPF_MEM_BIT;
    zpfinstargbits[ZPF_REG_UNIT][ZPF_ST] = zpfmergeargbits(sbits, dbits);
    sbits = ZPF_X_BIT;
    zpfinstargbits[ZPF_REG_UNIT][ZPF_STX] = zpfmergeargbits(sbits, dbits);
    dbits = ZPF_LABEL_BIT;
    sbits = 0;
    zpfinstargbits[ZPF_FLOW_UNIT][ZPF_JMP] = zpfmergeargbits(sbits, dbits);
    zpfinstargbits[ZPF_FLOW_UNIT][ZPF_JA] = zpfmergeargbits(sbits, dbits);
    sbits = ZPF_K_BIT;
    dbits = ZPF_JMPT_BIT | ZPF_JMP2_BIT;
    zpfinstargbits[ZPF_FLOW_UNIT][ZPF_JEQ] = zpfmergeargbits(sbits, dbits);
    dbits = ZPF_JMPT_BIT;
    zpfinstargbits[ZPF_FLOW_UNIT][ZPF_JNE] = zpfmergeargbits(sbits, dbits);
    zpfinstargbits[ZPF_FLOW_UNIT][ZPF_JNEQ] = zpfmergeargbits(sbits, dbits);
    zpfinstargbits[ZPF_FLOW_UNIT][ZPF_JLT] = zpfmergeargbits(sbits, dbits);
    zpfinstargbits[ZPF_FLOW_UNIT][ZPF_JLE] = zpfmergeargbits(sbits, dbits);
    dbits = ZPF_JMPT_BIT | ZPF_JMP2_BIT;
    zpfinstargbits[ZPF_FLOW_UNIT][ZPF_JGT] = zpfmergeargbits(sbits, dbits);
    zpfinstargbits[ZPF_FLOW_UNIT][ZPF_JGE] = zpfmergeargbits(sbits, dbits);
    zpfinstargbits[ZPF_FLOW_UNIT][ZPF_JSET] = zpfmergeargbits(sbits, dbits);
    sbits = ZPF_K_BIT | ZPF_ABIT;
    dbits = 0;
    zpfinstargbits[ZPF_FLOW_UNIT][ZPF_RET] = zpfmergeargbits(sbits, dbits);
    sbits = ZPF_A_BIT;
    dbits = ZPF_X_BIT;
    zpfinstargbits[ZPF_XFER_UNIT][ZPF_TAX] = zpfmergeargbits(sbits, dbits);
    sbits = ZPF_X_BIT;
    dbits = ZPF_A_BIT;
    zpfinstargbits[ZPF_XFER_UNIT][ZPF_TXA] = zpfmergeargbits(sbits, dbits);
    
    return;
}
    
long
zpmhashinst(struct zpminst *inst)
{
    const char *ptr = inst->name;
    long        shift = 2;
    long        key = 0;
    
    while (*ptr) {
        key += *ptr;
        key <<= shift;
        ptr++;
    }
    key &= (ZPM_NHASH - 1);

    return key;
}

void
zpminitinsttabs(void)
{
    struct zpfinst *inst;
    long            unit;
    long            op;
    long            key;

    for (unit = ZPF_ALU_UNIT ; unit < ZPF_NUNIT ; unit++) {
        inst = &zpfinsttab[unit][0];
        for (op = 0 ; op < ZPF_UNIT_NOP ; op++) {
            inst->name = strdup(zpfinstnametab[unit][op]);;
            inst->flg = zpfinstargbits[unit][op];
            inst->unit = unit;
            inst->inst = op;
            key = zpmhashinst(ptr);
            inst->key = key;
            inst->chain = zpminsthash[key];
            zpminsthash[key] = inst;
            inst++;
        }
    }
    
    return;
}

void
zpfinitdata(void)
{
    zpfinitinstnames();
    zpfinitinstargbits();
    zpfinitinsttabs();

    return;
}

