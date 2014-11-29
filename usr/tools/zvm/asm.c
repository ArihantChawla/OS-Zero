/* zero virtual machine assembler interface for zero assembler (zas) */

#include <zas/conf.h>

#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <zero/trix.h>
#include <zas/zas.h>
#include <zas/opt.h>
#include <zvm/zvm.h>
#include <zvm/op.h>

struct zasop  zvminsttab[ZVMNOP];
#if (ZVMMULTITAB)
struct zasop *zvmoptab[ZVMNOP];
#else
struct zasop *zvmophash[ZASNHASH];
#endif

void
zvminitopt(void)
{
#if (ZASALIGN)
    zasinitalign(sizeof(struct zvmopcode));
#endif

    return;
}

#if (ZVMMULTITAB)

/*
 * operation info structure addresses are stored in a multilevel table
 * - the top level table is indexed with the first byte of mnemonic and so on
 */
long
asmaddop(const uint8_t *str, struct zasop *op)
{
    long           key = *str++;
    uint8_t        len = (key) ? 1 : 0;
    struct zasop  *ptr1 = zvmoptab[key];
    struct zasop **ptr2;

    if (!ptr1) {
        ptr1 = calloc(sizeof(struct zasopinfo *), ZVMNOP);
        if (!ptr1) {
            fprintf(stderr, "failed to allocate operation table\n");
            
            return 0;
        }
        zvmoptab[key] = ptr1;
    }
    
    return 0;
}

struct zasop *
asmfindop(const uint8_t *str)
{
    long          key = *str++;
    struct zasop *ptr = zvmoptab[key];

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
asmaddop(const uint8_t *str, struct zasop *op)
{
//    uint8_t       *str = (uint8_t *)op->name;
    unsigned long key = 0;
    long          len = 0;

    while (isalpha(*str)) {
        key += *str++;
        len++;
    }
//    op->namelen = len;
    op->len = len;
    key &= (ZASNHASH - 1);
    op->next = zvmophash[key];
    zvmophash[key] = op;

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
    key &= (ZASNHASH - 1);
    op = zvmophash[key];
    while ((op) && strncmp((char *)op->name, (char *)name, op->len)) {
        op = op->next;
    }

    return op;
}

#endif /* ZVMMULTITAB */

zasuword_t
asmgetreg(uint8_t *str, zasword_t *retsize, uint8_t **retptr)
{
    zasuword_t reg = 0;
    zasword_t  size = 0;
    
#if (ZASDEBUG)
    fprintf(stderr, "getreg: %s\n", str);
#endif
#if (ZASNEWHACKS)
    if (*str == 'r') {
        str++;
#if (ZAS32BIT)
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
#if (!ZAS32BIT)
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
    struct zvmopcode *op = NULL;
    zasmemadr_t       opadr = adr;
    struct zastoken  *token1 = NULL;
    struct zastoken  *token2 = NULL;
    struct zastoken  *retval = NULL;
    struct zassymrec *sym;
    uint8_t           narg = token->data.inst.narg;
    uint8_t           len = sizeof(struct zvmopcode);

#if 0
    opadr = zasalignop(adr);
    while (adr < opadr) {
        zvm.physmem[adr] = ZVMOPNOP;
        adr++;
    }
#endif
#if (ZASDB)
    zasaddline(adr, token->data.inst.data, token->file, token->line);
#endif
    op = (struct zvmopcode *)&zvm.physmem[adr];
    op->code = token->data.inst.op;
    if (op->code == ZVMOPNOP) {
        retval = token->next;
        adr++;
    } else if (!narg) {
        op->reg1 = 0;
        op->reg2 = 0;
        retval = token->next;
    } else {
        token1 = token->next;
        zasfreetoken(token);
        if (token1) {
            switch(token1->type) {
                case ZASTOKENVALUE:
                    op->arg1t = ZVMARGIMMED;
                    op->args[0] = token1->data.value.val;
                    len += sizeof(zasword_t);
                    
                    break;
                case ZASTOKENREG:
                    op->arg1t = ZVMARGREG;
                    op->reg1 = token1->data.reg;
                    
                    break;
                case ZASTOKENSYM:
                    op->arg1t = ZVMARGADR;
                    sym = malloc(sizeof(struct zassymrec));
                    sym->name = (uint8_t *)strdup((char *)token1->data.sym.name);
                    sym->adr = (uintptr_t)&op->args[0];
                    zasqueuesym(sym);
                    len += sizeof(zasmemadr_t);
                    
                    break;
                case ZASTOKENINDIR:
                    token1 = token1->next;
                    if (token1->type == ZASTOKENREG) {
                        op->arg1t = ZVMARGREG;
                        op->reg1 = token1->data.reg;
                    } else {
                        fprintf(stderr, "indirect addressing requires a register\n");
                        
                        exit(1);
                    }
                    
                    break;
                case ZASTOKENIMMED:
                    op->arg1t = ZVMARGIMMED;
                    op->args[0] = token1->val;
                    len += sizeof(zasword_t);
                    
                    break;
                case ZASTOKENADR:
                    op->arg1t = ZVMARGIMMED;
                    sym = malloc(sizeof(struct zassymrec));
                    sym->name = (uint8_t *)strdup((char *)token1->data.sym.name);
                    sym->adr = (uintptr_t)&op->args[0];
                    zasqueuesym(sym);
                    len += sizeof(zasmemadr_t);
                    
                    break;
                case ZASTOKENINDEX:
                    op->arg1t = ZVMARGREG;
                    op->reg1 = token1->data.ndx.reg;
                    op->args[0] = token1->data.ndx.val;
                    len += sizeof(zasword_t);
                    
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
            switch(token2->type) {
                case ZASTOKENVALUE:
                    op->arg2t = ZVMARGIMMED;
                    if (op->arg1t == ZVMARGREG) {
                        op->args[0] = token2->data.value.val;
                    } else {
                        op->args[1] = token2->data.value.val;
                    }
                    len += sizeof(zasword_t);
                    
                    break;
                case ZASTOKENREG:
                    op->arg2t = ZVMARGREG;
                    op->reg2 = token2->data.reg;
                    
                    break;
                case ZASTOKENSYM:
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
                case ZASTOKENINDIR:
                    token2 = token2->next;
                    if (token2->type == ZASTOKENREG) {
                        op->arg2t = ZVMARGREG;
                        op->reg2 = token2->data.reg;
                    } else {
                        fprintf(stderr, "indirect addressing requires a register\n");
                        
                        exit(1);
                    }
                    
                    break;
                case ZASTOKENIMMED:
                    op->arg2t = ZVMARGIMMED;
                    if (op->arg1t == ZVMARGREG) {
                        op->args[0] = token2->val;
                    } else {
                        op->args[1] = token2->val;
                    }
                    len += sizeof(zasword_t);
                    
                    break;
                case ZASTOKENADR:
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
                case ZASTOKENINDEX:
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
                    zasprinttoken(token2);
                    
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

void
asmprintop(struct zvmopcode *op)
{
    ;
}

