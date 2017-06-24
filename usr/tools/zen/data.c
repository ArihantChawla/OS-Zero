#include <string.h>

struct zensym  *zensymhash[ZENNHASH];
struct zeninst *zeninsthash[ZENNHASH];
struct zeninst  zeninsttab[ZEN_NUNIT][ZEN_UNIT_NOP];
const char     *zeninstnametab[ZEN_NUNIT][ZEN_UNIT_NOP];
const char     *zeninstnametab[ZEN_NUNIT][ZEN_UNIT_NOP];
uint32_t        zeninstargbits[ZEN_NUNIT][ZEN_UNIT_NOP];
unsigned char   zenreqargsmap[256];
unsigned char   zennospacemap[256 / CHAR_BIT];
zenopfunc      *zenopfunctab[ZEN_OP_NUNIT][ZEN_OP_NINST];
zenop         **zenoptab;


void
zeninitinstnames(void)
{
    long            unit;
    
    unit = ZEN_ALU_UNIT;
    zeninstnametab[unit][ZEN_ADD] = strdup("add");
    zeninstnametab[unit][ZEN_SUB] = strdup("sub");
    zeninstnametab[unit][ZEN_MUL] = strdup("mul");
    zeninstnametab[unit][ZEN_DIV] = strdup("div");
    zeninstnametab[unit][ZEN_MOD] = strdup("mod");
    zeninstnametab[unit][ZEN_AND = strdup("and");
    zeninstnametab[unit][ZEN_OR] = strdup("or"); 
    zeninstnametab[unit][ZEN_XOR] = strdup("xor");
    zeninstnametab[unit][ZEN_LSH] = strdup("lsh");
    zeninstnametab[unit][ZEN_RSH] = strdup("rsh");
    zeninstnametab[unit][ZEN_NEG] = strdup("neg");
    unit = ZEN_REG_UNIT;
    zeninstnametab[unit][ZEN_LDB] = strdup("ldb");
    zeninstnametab[unit][ZEN_LDH] = strdup("ldh");
    zeninstnametab[unit][ZEN_LDI] = strdup("ldi");
    zeninstnametab[unit][ZEN_LD] = strdup("ld");
    zeninstnametab[unit][ZEN_LDX] = strdup("ldx");
    zeninstnametab[unit][ZEN_LDXI] = strdup("ldxi");
    zeninstnametab[unit][ZEN_LDXB] = strdup("ldxb");
    zeninstnametab[unit][ZEN_ST] = strdup("st");
    zeninstnametab[unit][ZEN_STX] = strdup("stx");
    unit = ZEN_FLOW_UNIT;
    zeninstnametab[unit][ZEN_JMP] = strdup("jmp");
    zeninstnametab[unit][ZEN_JA] = strdup("ja");
    zeninstnametab[unit][ZEN_JEQ] = strdup("jeq");
    zeninstnametab[unit][ZEN_JNE] = strdup("jne");
    zeninstnametab[unit][ZEN_JLT] = strdup("jlt");
    zeninstnametab[unit][ZEN_JLE] = strdup("jle");
    zeninstnametab[unit][ZEN_JGT] = strdup("jgt");
    zeninstnametab[unit][ZEN_JGE] = strdup("jge");
    zeninstnametab[unit][ZEN_JSET] = strdup("jset");
    zeninstnametab[unit][ZEN_RET] = strdup("ret");
    unit = ZEN_XFER_UNIT;
    zeninstnametab[unit][ZEN_TAX] = strdup("tax");
    zeninstnametab[unit][ZEN_TXA] = strdup("txa");
    
    return;
}
        
void
zeninitinstargbits(void)
{
    sbits = ZEN_X_BIT | ZEN_K_BIT;
    dbits = ZEN_A_BIT;
    zeninstargbits[ZEN_ALU_UNIT][ZEN_ADD] = zenmergeargbits(sbits, dbits);
    zeninstargbits[ZEN_ALU_UNIT][ZEN_SUB] = zenmergeargbits(sbits, dbits);
    zeninstargbits[ZEN_ALU_UNIT][ZEN_MUL] = zenmergeargbits(sbits, dbits);
    zeninstargbits[ZEN_ALU_UNIT][ZEN_DIV] = zenmergeargbits(sbits, dbits);
    zeninstargbits[ZEN_ALU_UNIT][ZEN_MOD] = zenmergeargbits(sbits, dbits);
    zeninstargbits[ZEN_ALU_UNIT][ZEN_AND] = zenmergeargbits(sbits, dbits);
    zeninstargbits[ZEN_ALU_UNIT][ZEN_OR] = zenmergeargbits(sbits, dbits);
    zeninstargbits[ZEN_ALU_UNIT][ZEN_XOR] = zenmergeargbits(sbits, dbits);
    zeninstargbits[ZEN_ALU_UNIT][ZEN_LSH] = zenmergeargbits(sbits, dbits);
    zeninstargbits[ZEN_ALU_UNIT][ZEN_RSH] = zenmergeargbits(sbits, dbits);
    sbits = ZEN_A_BIT;
    zeninstargbits[ZEN_ALU_UNIT][ZEN_NEG] = zenmergeargbits(sbits, dbits);
    sbits = ZEN_K_BIT | ZEN_OFS_BIT;
    dbits = ZEN_A_BIT;
    zeninstargbits[ZEN_REG_UNIT][ZEN_LDB] = zenmergeargbits(sbits, dbits);
    zeninstargbits[ZEN_REG_UNIT][ZEN_LDH] = zenmergeargbits(sbits, dbits);
    sbits = ZEN_K_BIT;
    dbits = ZEN_A_BIT;
    zeninstargbits[ZEN_REG_UNIT][ZEN_LDI] = zenmergeargbits(sbits, dbits);
    sbits = ZEN_K_BIT | ZEN_MEM_BIT | ZEN_PKT_BIT | ZEN_OFS_BIT;
    dbits = ZEN_A_BIT;
    zeninstargbits[ZEN_REG_UNIT][ZEN_LD] = zenmergeargbits(sbits, dbits);
    dbits = ZEN_X_BIT;
    zeninstargbits[ZEN_REG_UNIT][ZEN_LX] = zenmergeargbits(sbits, dbits);
    sbits = ZEN_K_BIT;
    zeninstargbits[ZEN_REG_UNIT][ZEN_LDXI] = zenmergeargbits(sbits, dbits);
    sbits = ZEN_PKT_BIT | ZEN_MUL4_BIT;
    zeninstargbits[ZEN_REG_UNIT][ZEN_LDXB] = zenmergeargbits(sbits, dbits);
    sbits = ZEN_A_BIT;
    dbits = ZEN_MEM_BIT;
    zeninstargbits[ZEN_REG_UNIT][ZEN_ST] = zenmergeargbits(sbits, dbits);
    sbits = ZEN_X_BIT;
    zeninstargbits[ZEN_REG_UNIT][ZEN_STX] = zenmergeargbits(sbits, dbits);
    dbits = ZEN_LABEL_BIT;
    sbits = 0;
    zeninstargbits[ZEN_FLOW_UNIT][ZEN_JMP] = zenmergeargbits(sbits, dbits);
    zeninstargbits[ZEN_FLOW_UNIT][ZEN_JA] = zenmergeargbits(sbits, dbits);
    sbits = ZEN_K_BIT;
    dbits = ZEN_JMPT_BIT | ZEN_JMP2_BIT;
    zeninstargbits[ZEN_FLOW_UNIT][ZEN_JEQ] = zenmergeargbits(sbits, dbits);
    dbits = ZEN_JMPT_BIT;
    zeninstargbits[ZEN_FLOW_UNIT][ZEN_JNE] = zenmergeargbits(sbits, dbits);
    zeninstargbits[ZEN_FLOW_UNIT][ZEN_JNEQ] = zenmergeargbits(sbits, dbits);
    zeninstargbits[ZEN_FLOW_UNIT][ZEN_JLT] = zenmergeargbits(sbits, dbits);
    zeninstargbits[ZEN_FLOW_UNIT][ZEN_JLE] = zenmergeargbits(sbits, dbits);
    dbits = ZEN_JMPT_BIT | ZEN_JMP2_BIT;
    zeninstargbits[ZEN_FLOW_UNIT][ZEN_JGT] = zenmergeargbits(sbits, dbits);
    zeninstargbits[ZEN_FLOW_UNIT][ZEN_JGE] = zenmergeargbits(sbits, dbits);
    zeninstargbits[ZEN_FLOW_UNIT][ZEN_JSET] = zenmergeargbits(sbits, dbits);
    sbits = ZEN_K_BIT | ZEN_ABIT;
    dbits = 0;
    zeninstargbits[ZEN_FLOW_UNIT][ZEN_RET] = zenmergeargbits(sbits, dbits);
    sbits = ZEN_A_BIT;
    dbits = ZEN_X_BIT;
    zeninstargbits[ZEN_XFER_UNIT][ZEN_TAX] = zenmergeargbits(sbits, dbits);
    sbits = ZEN_X_BIT;
    dbits = ZEN_A_BIT;
    zeninstargbits[ZEN_XFER_UNIT][ZEN_TXA] = zenmergeargbits(sbits, dbits);
    
    return;
}
    
long
zenhashinst(struct zeninst *inst)
{
    const char *ptr = inst->name;
    long        shift = 2;
    long        key = 0;
    
    while (*ptr) {
        key += *ptr;
        key <<= shift;
        ptr++;
    }
    key &= (ZEN_NHASH - 1);

    return key;
}

void
zeninitinsttabs(void)
{
    struct zeninst *inst;
    long            unit;
    long            op;
    long            key;

    for (unit = ZEN_ALU_UNIT ; unit < ZEN_NUNIT ; unit++) {
        inst = &zeninsttab[unit][0];
        for (op = 0 ; op < ZEN_UNIT_NOP ; op++) {
            inst->name = strdup(zeninstnametab[unit][op]);;
            inst->flg = zeninstargbits[unit][op];
            inst->unit = unit;
            inst->inst = op;
            key = zenhashinst(ptr);
            inst->key = key;
            inst->chain = zeninsthash[key];
            zeninsthash[key] = inst;
            inst++;
        }
    }
    
    return;
}

void
zeninitdata(void)
{
    zeninitinstnames();
    zeninitinstargbits();
    zeninitinsttabs();

    return;
}

