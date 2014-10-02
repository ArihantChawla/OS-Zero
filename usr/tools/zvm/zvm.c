#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <zero/param.h>
#include <zero/cdecl.h>
#include <zas/zas.h>
#include <zvm/zvm.h>

/* structure copy */
struct zasop      *zvmoptab[ZVMNOP] ALIGNED(PAGESIZE);
struct zasop       zvmopmtab[128];
static struct zvm  zvm;

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
    
    if (key) {
        while (key) {
            len++;
            ptr = pptr->tab;
            if (!ptr) {
                ptr = calloc(sizeof(struct zasopinfo), 128);
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
    zvm.physsize = len;

    return ptr;
}

void
zvminitasmop(uint8_t unit, uint8_t code, uint8_t *str, uint8_t narg)
{
    struct zasop *op = &zvmopmtab[(unit << 4) | code];
    
    op->name = str;
    op->code = code;
    op->narg = narg;
    if (!zvmaddasm(str, op)) {
        fprintf(stderr, "failed to initialise assembly operation:\n");
        fprintf(stderr, "unit == %d, code == %d, str == %s, narg = %d\n",
                (int)unit, (int)code, str, (int)narg);
        
        exit(1);
    }
}

void
zvminitasm(void)
{
    /* logical operations */
    zvminitasmop(ZVMOPLOGIC, ZVMOPNOT, "not", 1);
    zvminitasmop(ZVMOPLOGIC, ZVMOPAND, "and", 2);
    zvminitasmop(ZVMOPLOGIC, ZVMOPOR, "or", 2);
    zvminitasmop(ZVMOPLOGIC, ZVMOPXOR, "xor", 2);
    /* shift operations */
    zvminitasmop(ZVMOPSHIFT, ZVMOPSHR, "shr", 2);
    zvminitasmop(ZVMOPSHIFT, ZVMOPSAR, "sar", 2);
    zvminitasmop(ZVMOPSHIFT, ZVMOPSHL, "shl", 2);
    zvminitasmop(ZVMOPSHIFT, ZVMOPROR, "ror", 2);
    zvminitasmop(ZVMOPSHIFT, ZVMOPROL, "rol", 2);
    /* arithmetic operations */
    zvminitasmop(ZVMOPARITH, ZVMOPINC, "inc", 1);
    zvminitasmop(ZVMOPARITH, ZVMOPDEC, "dec", 1);
    zvminitasmop(ZVMOPARITH, ZVMOPADD, "add", 2);
    zvminitasmop(ZVMOPARITH, ZVMOPSUB, "sub", 2);
    zvminitasmop(ZVMOPARITH, ZVMOPCMP, "cmp", 2);
    zvminitasmop(ZVMOPARITH, ZVMOPMUL, "mul", 2);
    zvminitasmop(ZVMOPARITH, ZVMOPDIV, "div", 2);
    zvminitasmop(ZVMOPARITH, ZVMOPMOD, "mod", 2);
    /* branch operations */
    zvminitasmop(ZVMOPBRANCH, ZVMOPJMP, "jmp", 1);
    zvminitasmop(ZVMOPBRANCH, ZVMOPBZ, "bz", 1);
    zvminitasmop(ZVMOPBRANCH, ZVMOPBNZ, "bnz", 1);
    zvminitasmop(ZVMOPBRANCH, ZVMOPBLT, "blt", 1);
    zvminitasmop(ZVMOPBRANCH, ZVMOPBLE, "ble", 1);
    zvminitasmop(ZVMOPBRANCH, ZVMOPBGT, "bgt", 1);
    zvminitasmop(ZVMOPBRANCH, ZVMOPBGE, "bge", 1);
    zvminitasmop(ZVMOPBRANCH, ZVMOPBO, "bo", 1);
    zvminitasmop(ZVMOPBRANCH, ZVMOPBNO, "bno", 1);
    zvminitasmop(ZVMOPBRANCH, ZVMOPBC, "bc", 1);
    zvminitasmop(ZVMOPBRANCH, ZVMOPBNC, "bnc", 1);
    /* stack operations */
    zvminitasmop(ZVMOPSTACK, ZVMOPPOP, "pop", 1);
    zvminitasmop(ZVMOPSTACK, ZVMOPPUSH, "push", 1);
    zvminitasmop(ZVMOPSTACK, ZVMOPPUSHA, "pusha", 0);
    /* load/store */
    zvminitasmop(ZVMOPLDSTR, ZVMOPMOV, "mov", 2);
    zvminitasmop(ZVMOPLDSTR, ZVMOPMOVB, "movb", 2);
    zvminitasmop(ZVMOPLDSTR, ZVMOPMOVW, "movw", 2);
    zvminitasmop(ZVMOPLDSTR, ZVMOPMOVQ, "movq", 2);
    /* function calls */
    zvminitasmop(ZVMOPFUNC, ZVMOPCALL, "call", 1);
    zvminitasmop(ZVMOPFUNC, ZVMOPENTER, "enter", 1);
    zvminitasmop(ZVMOPFUNC, ZVMOPLEAVE, "leave", 1);
    zvminitasmop(ZVMOPFUNC, ZVMOPRET, "ret", 1);
    /* machine status word */
    zvminitasmop(ZVMOPMSW, ZVMOPLMSW, "lmsw", 1);
    zvminitasmop(ZVMOPMSW, ZVMOPSMSW, "smsw", 1);
    /* machine state */
    zvminitasmop(ZVMOPMACH, ZVMOPRESET, "reset", 0);
    zvminitasmop(ZVMOPMACH, ZVMOPHLT, "hlt", 0);
    
    return;
};

void
zvminit(size_t memsize)
{
    zasinitmem(memsize);
    zvminitasm();
}

static struct zastoken *
zasprocinst(struct zastoken *token, zasmemadr_t adr,
            zasmemadr_t *retadr)
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
    {
        op = (struct zvmopcode *)&zvm.physmem[adr];
        op->inst = token->data.inst.op;
        if (op->inst == ZVMOPNOP) {
            retval = token->next;
            adr++;
        } else
            if (!narg) {
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
                            len += 4;
                            
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
                            len += 4;
                            
                            break;
                        case TOKENADR:
                            op->arg1t = ZVMARGIMMED;
                            sym = malloc(sizeof(struct zassymrec));
                            sym->name = (uint8_t *)strdup((char *)token1->data.sym.name);
                            sym->adr = (uintptr_t)&op->args[0];
                            zasqueuesym(sym);
                            len += 4;

                            break;
                        case TOKENINDEX:
                            op->arg1t = ZVMARGREG;
                            op->reg1 = token1->data.ndx.reg;
                            op->args[0] = token1->data.ndx.val;
                            len += 4;
                            
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
                            len += 4;
                            
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
                            len += 4;
                            
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
                            len += 4;
                            
                            break;
                        case TOKENINDEX:
                            op->arg2t = ZVMARGREG;
                            op->reg2 = token2->data.ndx.reg;
                            if (op->arg1t == ZVMARGREG) {
                                op->args[0] = token2->data.ndx.val;
                            } else {
                                op->args[1] = token2->data.ndx.val;
                            }
                            len += 4;
                            
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
    }
    *retadr = adr + len;

    return retval;
}

