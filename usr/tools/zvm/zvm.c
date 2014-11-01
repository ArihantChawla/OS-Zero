#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <zero/param.h>
#include <zero/cdecl.h>
#include <zero/trix.h>
#include <zas/zas.h>
#include <zvm/zvm.h>
#include <zvm/op.h>

extern struct zastoken *zastokenqueue;
extern unsigned long    zasinputread;
extern zasmemadr_t      _startadr;

zvmopfunc_t  *zvmopfunctab[ZVMNOP];
struct zasop *zvmoptab[ZVMNOP] ALIGNED(PAGESIZE);
struct zasop  zvmopmtab[ZVMNOP];
struct zvm    zvm;

/*
 * operation info structure addresses are stored in a multilevel table
 * - the top level table is indexed with the first byte of mnemonic and so on
 */
struct zasop *
zvmaddasm(const uint8_t *str, struct zasop *op)
{
    long          key = *str++;
    uint8_t       len = 0;
    struct zasop *pptr = zvmoptab[key];
    struct zasop *ptr = NULL;

    if (!pptr) {
        ptr = calloc(sizeof(struct zasopinfo), ZVMNOP);
        if (!ptr) {
            fprintf(stderr, "failed to allocate operation info\n");
            
            return NULL;
        }
        zvmoptab[key] = ptr;
        pptr = ptr;
    }
    if (key) {
        while (key) {
            len++;
            ptr = pptr->tab;
            if (!ptr) {
                ptr = calloc(sizeof(struct zasopinfo), ZVMNOP);
                if (!ptr) {
                    fprintf(stderr, "failed to allocate operation info\n");
            
                    return NULL;
                }
                pptr->tab = ptr;
            }
            pptr = &ptr[key];
            key = *str++;
        }
        *((struct zasop **)pptr) = op;
        op->len = len;
        
        return op;
    }
    
    return NULL;
}

struct zasop *
zvmfindasm(const uint8_t *str)
{
    long          key = *str++;
    struct zasop *ptr = zvmoptab[key];

    if (key) {
        while (key) {   
            if (!*str) {
        
                return ptr;
            } else if (ptr->tab) {
                key = *str++;
                ptr = &ptr->tab[key];
            } else {
            
                return NULL;
            }
        }
        
        return ptr;
    }
    
    return NULL;
}

void *
zvminitmem(long memsize)
{
    size_t len = ZASMEMSIZE;
    void *ptr = malloc(ZASMEMSIZE);

    while (!ptr) {
        len >>= 1;
        ptr = malloc(len);
    }
    zvm.memsize = len;

    return ptr;
}

void
zvminitasmop(uint8_t unit, uint8_t inst, uint8_t *str, uint8_t narg)
{
    struct zasop *op = &zvmopmtab[(unit << 4) | inst];
    
    op->name = str;
    op->code = inst;
    op->narg = narg;
    if (!zvmaddasm(str, op)) {
        fprintf(stderr, "failed to initialise assembly operation:\n");
        fprintf(stderr, "unit == %d, inst == %d, str == %s, narg = %d\n",
                (int)unit, (int)inst, str, (int)narg);
        
        exit(1);
    }
}

void
zvminitasm(void)
{
    /* logical operations */
    zvminitasmop(ZVMOPLOGIC, ZVMOPNOT, (uint8_t *)"not", 1);
    zvminitasmop(ZVMOPLOGIC, ZVMOPAND, (uint8_t *)"and", 2);
    zvminitasmop(ZVMOPLOGIC, ZVMOPOR, (uint8_t *)"or", 2);
    zvminitasmop(ZVMOPLOGIC, ZVMOPXOR, (uint8_t *)"xor", 2);
    /* shift operations */
    zvminitasmop(ZVMOPSHIFT, ZVMOPSHR, (uint8_t *)"shr", 2);
    zvminitasmop(ZVMOPSHIFT, ZVMOPSAR, (uint8_t *)"sar", 2);
    zvminitasmop(ZVMOPSHIFT, ZVMOPSHL, (uint8_t *)"shl", 2);
    zvminitasmop(ZVMOPSHIFT, ZVMOPROR, (uint8_t *)"ror", 2);
    zvminitasmop(ZVMOPSHIFT, ZVMOPROL, (uint8_t *)"rol", 2);
    /* arithmetic operations */
    zvminitasmop(ZVMOPARITH, ZVMOPINC, (uint8_t *)"inc", 1);
    zvminitasmop(ZVMOPARITH, ZVMOPDEC, (uint8_t *)"dec", 1);
    zvminitasmop(ZVMOPARITH, ZVMOPADD, (uint8_t *)"add", 2);
    zvminitasmop(ZVMOPARITH, ZVMOPSUB, (uint8_t *)"sub", 2);
    zvminitasmop(ZVMOPARITH, ZVMOPCMP, (uint8_t *)"cmp", 2);
    zvminitasmop(ZVMOPARITH, ZVMOPMUL, (uint8_t *)"mul", 2);
    zvminitasmop(ZVMOPARITH, ZVMOPDIV, (uint8_t *)"div", 2);
    zvminitasmop(ZVMOPARITH, ZVMOPMOD, (uint8_t *)"mod", 2);
    /* branch operations */
    zvminitasmop(ZVMOPBRANCH, ZVMOPJMP, (uint8_t *)"jmp", 1);
    zvminitasmop(ZVMOPBRANCH, ZVMOPBZ, (uint8_t *)"bz", 1);
    zvminitasmop(ZVMOPBRANCH, ZVMOPBNZ, (uint8_t *)"bnz", 1);
    zvminitasmop(ZVMOPBRANCH, ZVMOPBLT, (uint8_t *)"blt", 1);
    zvminitasmop(ZVMOPBRANCH, ZVMOPBLE, (uint8_t *)"ble", 1);
    zvminitasmop(ZVMOPBRANCH, ZVMOPBGT, (uint8_t *)"bgt", 1);
    zvminitasmop(ZVMOPBRANCH, ZVMOPBGE, (uint8_t *)"bge", 1);
    zvminitasmop(ZVMOPBRANCH, ZVMOPBO, (uint8_t *)"bo", 1);
    zvminitasmop(ZVMOPBRANCH, ZVMOPBNO, (uint8_t *)"bno", 1);
    zvminitasmop(ZVMOPBRANCH, ZVMOPBC, (uint8_t *)"bc", 1);
    zvminitasmop(ZVMOPBRANCH, ZVMOPBNC, (uint8_t *)"bnc", 1);
    /* stack operations */
    zvminitasmop(ZVMOPSTACK, ZVMOPPOP, (uint8_t *)"pop", 1);
    zvminitasmop(ZVMOPSTACK, ZVMOPPUSH, (uint8_t *)"push", 1);
    zvminitasmop(ZVMOPSTACK, ZVMOPPUSHA, (uint8_t *)"pusha", 0);
    /* load/store */
    zvminitasmop(ZVMOPLDSTR, ZVMOPMOVL, (uint8_t *)"mov", 2);
    zvminitasmop(ZVMOPLDSTR, ZVMOPMOVB, (uint8_t *)"movb", 2);
    zvminitasmop(ZVMOPLDSTR, ZVMOPMOVW, (uint8_t *)"movw", 2);
    zvminitasmop(ZVMOPLDSTR, ZVMOPMOVQ, (uint8_t *)"movq", 2);
    /* function calls */
    zvminitasmop(ZVMOPFUNC, ZVMOPCALL, (uint8_t *)"call", 1);
    zvminitasmop(ZVMOPFUNC, ZVMOPENTER, (uint8_t *)"enter", 1);
    zvminitasmop(ZVMOPFUNC, ZVMOPLEAVE, (uint8_t *)"leave", 1);
    zvminitasmop(ZVMOPFUNC, ZVMOPRET, (uint8_t *)"ret", 1);
    /* machine status word */
    zvminitasmop(ZVMOPMSW, ZVMOPLMSW, (uint8_t *)"lmsw", 1);
    zvminitasmop(ZVMOPMSW, ZVMOPSMSW, (uint8_t *)"smsw", 1);
    /* machine state */
    zvminitasmop(ZVMOPMACH, ZVMOPRESET, (uint8_t *)"reset", 0);
    zvminitasmop(ZVMOPMACH, ZVMOPHLT, (uint8_t *)"hlt", 0);
    
    return;
};

void
zvminit(size_t memsize)
{
    zvminitmem(memsize);
    zvminitasm();
}

void *
zvmloop(zasmemadr_t _startadr)
{
    zvmopfunc_t      *func;
    struct zvmopcode *op;
#if (ZVMTRACE)
    int               i;
#endif
#if (ZVMDB)
    struct zasline *line;
#endif

#if (ZVMTRACE) && 0
    fprintf(stderr, "memory\n");
    fprintf(stderr, "------\n");
    for (i = ZASTEXTBASE ; i < ZASTEXTBASE + 256 ; i++) {
        fprintf(stderr, "%02x ", physmem[i]);
    }
    fprintf(stderr, "\n");
    fprintf(stderr, "registers\n");
    fprintf(stderr, "---------\n");
    fprintf(stderr, "---------\n");
    for (i = 0 ; i < NREG ; i++) {
        fprintf(stderr, "r%d:\t%x\n", i, zvm.regs[i]);
    }
#endif
    zvm.shutdown = 0;
//    memcpy(&zvm.cpustat, cpustat, sizeof(struct zvmcpustate));
//    free(cpustat);
    while (!zvm.shutdown) {
        op = (struct zvmopcode *)&zvm.physmem[zvm.pc];
        if (op->inst == ZVMOPNOP) {
            zvm.pc += 4;
        } else {
//            zvm.cpustat.pc = rounduppow2(zvm.pc, sizeof(zasword_t));
            op = (struct zvmopcode *)&zvm.physmem[zvm.pc];
            func = zvmopfunctab[op->inst];
#if (ZVMTRACE)
            zvmprintop(op);
#endif
            if (func) {
#if (ZVMDB)
                line = zasfindline(zvm.pc);
                if (line) {
                    fprintf(stderr, "%s:%ld:\t%s\n", line->file, line->num, line->data);
                }
#endif
                func(op);
            } else {
                fprintf(stderr, "illegal instruction, PC == %lx\n",
                        (long)zvm.pc);
#if (ZVM)
//                zvmprintop(op);
#endif
                
                exit(1);
            }
        }
    }
#if (ZVMTRACE)
    fprintf(stderr, "memory\n");
    fprintf(stderr, "------\n");
    for (i = ZASTEXTBASE ; i < ZASTEXTBASE + 256 ; i++) {
        fprintf(stderr, "%02x ", physmem[i]);
    }
    fprintf(stderr, "\n");
    fprintf(stderr, "registers\n");
    fprintf(stderr, "---------\n");
    for (i = 0 ; i < NREG ; i++) {
        fprintf(stderr, "r%d:\t%x\n", i, zvm.regs[i]);
    }
#endif

    return NULL;
}

int
zvmmain(int argc, char *argv[])
{
    long        l;
    zasmemadr_t adr = ZASTEXTBASE;
#if (ZASPROF)
    PROFDECLCLK(clk);
#endif

    if (argc < 2) {
        fprintf(stderr, "usage: zvm <file1> ...\n");

        exit(1);
    }
    memset(zvm.physmem, 0, ZASTEXTBASE);
#if (ZASPROF)
    profstartclk(clk);
#endif
    for (l = 1 ; l < argc ; l++) {
#if (ZASBUF)
        zasreadfile(argv[l], adr, readbufcur);
#else
        zasreadfile(argv[l], adr);
#endif
        if (!zastokenqueue) {
            fprintf(stderr, "WARNING: no input in %s\n", argv[l]);
        } else {
            zasinputread = 1;
            adr = zastranslate(adr);
            zasresolve(ZASTEXTBASE);
            zasremovesyms();
#if (ZASPROF)
            profstopclk(clk);
            fprintf(stderr, "%ld microseconds to process %s\n",
                    profclkdiff(clk), argv[l]);
#endif        
        }
    }
    if (!zasinputread) {
        fprintf(stderr, "empty input\n");

        exit(1);
    }
    fprintf(stderr, "START: %lx\n", (long)_startadr);
//    wpminitthr(_startadr);
    zvmloop(_startadr);

    /* NOTREACHED */
    exit(0);
}

struct zastoken *
zasprocinst(struct zastoken *token, zasmemadr_t adr, zasmemadr_t *retadr)
{
    struct zvmopcode *op = NULL;

    zasmemadr_t       opadr = rounduppow2(adr, 4);
    struct zastoken  *token1 = NULL;
    struct zastoken  *token2 = NULL;
    struct zastoken  *retval = NULL;
    struct zassymrec *sym;
    uint8_t           narg = token->data.inst.narg;
//    uint8_t           len = token->data.inst.op == ZVMOPNOP ? 1 : 4;
    uint8_t           len = 4;

    while (adr < opadr) {
        zvm.physmem[adr] = ZVMOPNOP;
        adr++;
    }
//    adr = opadr;
#if (ZASDB)
    zasaddline(adr, token->data.inst.data, token->file, token->line);
#endif
    op = (struct zvmopcode *)&zvm.physmem[adr];
    op->inst = token->data.inst.op;
    if (op->inst == ZVMOPNOP) {
        retval = token->next;
        adr++;
    } else if (!narg) {
#if 0
        op->arg1t = ZVMARGNONE;
        op->arg2t = ZVMARGNONE;
#endif
        op->reg1 = 0;
        op->reg2 = 0;
        retval = token->next;
    } else {
        token1 = token->next;
        zasfreetoken(token);
        if (token1) {
            switch(token1->type) {
                case TOKENVALUE:
                    op->arg1t = ZVMARGIMMED;
                    op->args[0] = token1->data.value.val;
                    len += sizeof(zasword_t);
                    
                    break;
                case TOKENREG:
                    op->arg1t = ZVMARGREG;
                    op->reg1 = token1->data.reg;
                    
                    break;
                case TOKENSYM:
                    op->arg1t = ZVMARGADR;
                    sym = malloc(sizeof(struct zassymrec));
                    sym->name = (uint8_t *)strdup((char *)token1->data.sym.name);
                    sym->adr = (uintptr_t)&op->args[0];
                    zasqueuesym(sym);
                    len += sizeof(zasmemadr_t);
                    
                    break;
                case TOKENINDIR:
                    token1 = token1->next;
                    if (token1->type == TOKENREG) {
                        op->arg1t = ZVMARGREG;
                        op->reg1 = token1->data.reg;
                    } else {
                        fprintf(stderr, "indirect addressing requires a register\n");
                        
                        exit(1);
                    }
                    
                    break;
                case TOKENIMMED:
                    op->arg1t = ZVMARGIMMED;
                    op->args[0] = token1->val;
                    len += sizeof(zasword_t);
                    
                    break;
                case TOKENADR:
                    op->arg1t = ZVMARGIMMED;
                    sym = malloc(sizeof(struct zassymrec));
                    sym->name = (uint8_t *)strdup((char *)token1->data.sym.name);
                    sym->adr = (uintptr_t)&op->args[0];
                    zasqueuesym(sym);
                    len += sizeof(zasmemadr_t);
                    
                    break;
                case TOKENINDEX:
                    op->arg1t = ZVMARGREG;
                    op->reg1 = token1->data.ndx.reg;
                    op->args[0] = token1->data.ndx.val;
                    len += sizeof(zasword_t);
                    
                    break;
                default:
                    fprintf(stderr, "invalid argument 1 of type %lx\n", token1->type);
                    printtoken(token1);
                    
                    exit(1);
                    
                    break;
            }
            token2 = token1->next;
            zasfreetoken(token1);
            retval = token2;
        }
        if (narg == 1) {
//                    op->arg2t = ZVMARGNONE;
        } else if (narg == 2 && (token2)) {
            switch(token2->type) {
                case TOKENVALUE:
                    op->arg2t = ZVMARGIMMED;
                    if (op->arg1t == ZVMARGREG) {
                        op->args[0] = token2->data.value.val;
                    } else {
                        op->args[1] = token2->data.value.val;
                    }
                    len += sizeof(zasword_t);
                    
                    break;
                case TOKENREG:
                    op->arg2t = ZVMARGREG;
                    op->reg2 = token2->data.reg;
                    
                    break;
                case TOKENSYM:
                    op->arg2t = ZVMARGADR;
                    sym = malloc(sizeof(struct zassymrec));
                    sym->name = (uint8_t *)strdup((char *)token2->data.sym.name);
                    if (op->arg1t == ZVMARGREG) {
                        sym->adr = (uintptr_t)&op->args[0];
                    } else {
                        sym->adr = (uintptr_t)&op->args[1];
                    }
                    zasqueuesym(sym);
                    len += sizeof(zasmemadr_t);
                    
                    break;
                case TOKENINDIR:
                    token2 = token2->next;
                    if (token2->type == TOKENREG) {
                        op->arg2t = ZVMARGREG;
                        op->reg2 = token2->data.reg;
                    } else {
                        fprintf(stderr, "indirect addressing requires a register\n");
                        
                        exit(1);
                    }
                    
                    break;
                case TOKENIMMED:
                    op->arg2t = ZVMARGIMMED;
                    if (op->arg1t == ZVMARGREG) {
                        op->args[0] = token2->val;
                    } else {
                        op->args[1] = token2->val;
                    }
                    len += sizeof(zasword_t);
                    
                    break;
                case TOKENADR:
                    op->arg2t = ZVMARGIMMED;
                    sym = malloc(sizeof(struct zassymrec));
                    sym->name = (uint8_t *)strdup((char *)token2->data.sym.name);
                    if (op->arg1t == ZVMARGREG) {
                        sym->adr = (uintptr_t)&op->args[0];
                    } else {
                        sym->adr = (uintptr_t)&op->args[1];
                    }
                    zasqueuesym(sym);
                    len += sizeof(zasmemadr_t);
                    
                    break;
                case TOKENINDEX:
                    op->arg2t = ZVMARGREG;
                    op->reg2 = token2->data.ndx.reg;
                    if (op->arg1t == ZVMARGREG) {
                        op->args[0] = token2->data.ndx.val;
                    } else {
                        op->args[1] = token2->data.ndx.val;
                    }
                    len += sizeof(zasword_t);
                    
                    break;
                default:
                    fprintf(stderr, "invalid argument 2 of type %lx\n", token2->type);
                    printtoken(token2);
                    
                    exit(1);
                    
                    break;
            }
            retval = token2->next;
            zasfreetoken(token2);
        }
    }
    op->size = len >> 2;
    *retadr = adr + len;

    return retval;
}

int
main(int argc, char *argv[])
{
    zasinit(NULL, NULL);
    zvminit(ZASMEMSIZE);

    exit(zvmmain(argc, argv));
}

