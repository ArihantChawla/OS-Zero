/* zero virtual machine assembler interface for zero assembler (zas) */

#include <zas/conf.h>

#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <zero/trix.h>
#include <zas/zas.h>
#include <zas/opt.h>
#include <zpm/zpm.h>
#include <zpm/op.h>

struct zasop  zpminsttab[ZPMNOP];
#if (ZPMMULTITAB)
struct zasop *zpmoptab[ZPMNOP];
#else
struct zasop *zpmophash[ZASNHASH];
#endif

void
zpminitopt(void)
{
#if (ZPMALIGN)
    zasinitalign();
#endif

    return;
}

#if (ZPMMULTITAB)

/*
 * operation info structure addresses are stored in a multilevel table
 * - the top level table is indexed with the first byte of mnemonic and so on
 */
long
asmaddop(const char *str, struct zasop *op)
{
    long           key = *str++;
    uint8_t        len = (key) ? 1 : 0;
    struct zasop  *ptr1 = zpmoptab[key];
    struct zasop **ptr2;

    if (!ptr1) {
        ptr1 = calloc(sizeof(struct zasopinfo *), ZPMNOP);
        if (!ptr1) {
            fprintf(stderr, "failed to allocate operation table\n");
            
            return 0;
        }
        zpmoptab[key] = ptr1;
    }
    
    return 0;
}

struct zasop *
asmfindop(const char *str)
{
    long          key = *str++;
    struct zasop *ptr = zpmoptab[key];

    if (key) {
        while (key && isalpha(*str)) {   
            if ((ptr) && (ptr->tab)) {
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

#else

long
asmaddop(const char *str, struct zasop *op)
{
//    uint8_t       *str = op->name;
    unsigned long key = 0;
    long          len = 0;

    while (isalpha(*str)) {
        key += *str++;
        len++;
    }
//    op->namelen = len;
    op->len = len;
    key &= (ZASNHASH - 1);
    op->next = zpmophash[key];
    zpmophash[key] = op;

    return len;
}

struct zasop *
asmfindop(uint8_t *name)
{
    struct zasop  *op = NULL;
    uint8_t       *str = name;
    unsigned long  key = 0;

    while ((*str) && isalpha(*str)) {
        key += *str++;
    }
    key &= (ZASNHASHddddddddddddddddddddddddddddddddddddddddddddddddd - 1);
    op = zpmophash[key];
    while ((op) && strncmp((char *)op->name, (char *)name, op->len)) {
        op = op->next;
    }

    return op;
}

#endif /* ZPMMULTITAB */

zasuword_t
asmgetreg(uint8_t *str, zasword_t *retsize, uint8_t **retptr)
{
    zasuword_t reg = 0;
    zasword_t  size = 0;
    
#if (ZPMDEBUG)
    fprintf(stderr, "getreg: %s\n", str);
#endif
#if (ZPMNEWHACKS)
    if (*str == 'r') {
        str++;
#if (ZPM32BIT)
        size = 4;
#else
        size = 8;
#endif
    } else if (*str == 'b') {
        str++;
        size = 1;
    } else if (*str == 'w') {
        str++;
        size = 2;
    } else if (*str == 'l') {
        str++;
        size = 4;
#if (!ZPM32BIT)
    } else if (*str == 'q') {
        str++;
        size = 8;
#endif
    }
    if (size) {
        while ((*str) && isdigit(*str)) {
            reg *= 10;
            reg += *str - '0';
            str++;
        }
        while (*str == ')' || *str == ',') {
            str++;
        }
        *retsize = size;
        *retptr = str;
    } else {
        fprintf(stderr, "invalid register name %s\n", str);
        
        exit(1);
    }
#else
    if (*str == 'r') {
        str++;
        while ((*str) && isdigit(*str)) {
            reg *= 10;
            reg += *str - '0';
            str++;
        }
        while (*str == ')' || *str == ',') {
            str++;
        }
        *retptr = str;
    } else {
        fprintf(stderr, "invalid register name %s\n", str);
        
        exit(1);
    }
#endif
#if (WPMVEC)
    reg |= flg;
#endif

    return reg;
}

struct zastoken *
asmprocinst(struct zastoken *token, zasmemadr_t adr, zasmemadr_t *retadr)
{
    struct zpmopcode *op = NULL;
    zasmemadr_t       opadr = adr;
    struct zastoken  *token1 = NULL;
    struct zastoken  *token2 = NULL;
    struct zastoken  *retval = NULL;
    zasmemadr_t       argsz = 0;
    struct zassymrec *sym;
    zasuword_t        uval;
    uint8_t           narg = token->data.inst.narg;
    uint8_t           len = sizeof(struct zpmopcode);

#if 0
    opadr = zasalignop(adr);
    while (adr < opadr) {
        zpm.physmem[adr] = ZPMOPNOP;
        adr++;
    }
#endif
#if (ZPMDB)
    zasaddline(adr, token->data.inst.data, token->file, token->line);
#endif
    op = (struct zpmopcode *)&zpm.physmem[adr];
    op->code = token->data.inst.op;
    if (op->code == ZPMOPNOP) {
        retval = token->next;
    } else if (!narg) {
        op->reg1 = 0;
        op->reg2 = 0;
        retval = token->next;
    } else {
        token1 = token->next;
        zasfreetoken(token);
        if (token1) {
            switch (token1->type) {
                case ZASTOKENVALUE:
                    /* FIXME: do NOT allow this for STR */
                    uval = *((uint8_t *)&token1->data.value.val);
                    if (val <= 0xff) {
                        op->adrmode |= ZPM_IMM8_VAL;
                        op->imm8 = uval;
                    } else {
                        op->adrmode |= ZPM_IMM_VAL;
                        op->args[0] = val;
                        argsz = sizeof(zasword_t);
                    }
                    
                    break;
                case ZASTOKENREG:
                    op->adrmode |= ZPM_REG_VAL;
                    op->reg1 = token1->data.reg;
                    
                    break;
                case ZASTOKENSYM:
                    op->adrmode |= ZPM_IMM_ADR;
                    sym = malloc(sizeof(struct zassymrec));
                    sym->name = strdup((char *)token1->data.sym.name);
                    sym->adr = (uintptr_t)&op->args[0];
                    zasqueuesym(sym);
                    argsz = sizeof(zasmemadr_t);
                    
                    break;
                case ZASTOKENINDIR:
                    token1 = token1->next;
                    if (token1->type == ZASTOKENREG) {
                        op->adrmode |= ZPM_REG_VAL | ZPM_REG_INDIR;
                        op->reg1 = token1->data.reg;
                    } else {
                        fprintf(stderr, "indirect addressing requires a register\n");
                        
                        exit(1);
                    }
                    
                    break;
                case ZASTOKENIMMED:
                    uval = *((uint8_t *)&token1->val);
                    if (val <= 0xff) {
                        op->adrmode |= ZPM_IMM8_VAL;
                        op->imm8 = val;
                    } else {
                        op->adrmode |= ZPM_IMM_VAL;
                        op->args[0] = token1->val;
                        argsz = sizeof(zasword_t);
                    }
                    
                    break;
                case ZASTOKENADR:
//                    op->arg1t = ZPMARGIMMED;
                    op->adrmode |= ZPM_IMM_ADR;
                    sym = malloc(sizeof(struct zassymrec));
                    sym->name = strdup((char *)token1->data.sym.name);
                    sym->adr = (uintptr_t)&op->args[0];
                    zasqueuesym(sym);
                    argsz = sizeof(zasmemadr_t);
                    
                    break;
                case ZASTOKENINDEX:
//                    op->arg1t = ZPMARGREG;
                    op->adrmode |= ZPM_REG_VAL;
                    op->reg1 = token1->data.ndx.reg;
                    op->args[0] = token1->data.ndx.val;
                    argsz = sizeof(zasword_t);
                    
                    break;
                default:
                    fprintf(stderr, "invalid argument 1 of type %lx\n", token1->type);
                    zasprinttoken(token1);
                    
                    exit(1);
                    
                    break;
            }
            token2 = token1->next;
            zasfreetoken(token1);
            retval = token2;
        }
        if (narg == 2 && (token2)) {
            switch (token2->type) {
                case ZASTOKENVALUE:
                    op_>adrmode |= ZPM_IMM_ADR;
                    op->args[0] = token2->data.value.val;
                    argsz = sizeof(zasword_t);
                    
                    break;
                case ZASTOKENREG:
                    op->reg2 = token2->data.reg;
                    
                    break;
                case ZASTOKENSYM:
                    op->adrmode |= ZPM_IMM_ADR;
                    sym = malloc(sizeof(struct zassymrec));
                    sym->name = strdup((char *)token2->data.sym.name);
                    sym->adr = (uintptr_t)&op->args[0];
                    zasqueuesym(sym);
                    argsz = sizeof(zasmemadr_t);
                    
                    break;
                case ZASTOKENINDIR:
                    token2 = token2->next;
                    if (token2->type == ZASTOKENREG) {
                        op->adrmode |= ZPM_REG_INDIR;
                        op->reg2 = token2->data.reg;
                    } else {
                        fprintf(stderr, "indirect addressing requires a register\n");
                        
                        exit(1);
                    }
                    
                    break;
                case ZASTOKENIMMED:
                    op->adrmode |= ZPM_IMM_ADR;
                    op->args[0] = token2->val;
                    argsz = sizeof(zasword_t);
                    
                    break;
                case ZASTOKENADR:
                    op->adrmode |= ZPM_IMM_ADR;
                    sym = malloc(sizeof(struct zassymrec));
                    sym->name = strdup((char *)token2->data.sym.name);
                    sym->adr = (uintptr_t)&op->args[0];
                    zasqueuesym(sym);
                    argsz = sizeof(zasmemadr_t);
                    
                    break;
                case ZASTOKENINDEX:
                    op->adrmode |= ZPM_REG_NDX;
                    op->reg2 = token2->data.ndx.reg;
                    op->args[0] = token2->data.ndx.val;
                    argsz = sizeof(zasword_t);
                    
                    break;
                default:
                    fprintf(stderr, "invalid argument 2 of type %lx\n", token2->type);
                    zasprinttoken(token2);
                    
                    exit(1);
                    
                    break;
            }
            retval = token2->next;
            zasfreetoken(token2);
        }
    }
    op->argsz = argsz;
    *retadr = adr + len + argsz;
    
    return retval;
}

void
asmprintop(struct zpmopcode *op)
{
    ;
}

