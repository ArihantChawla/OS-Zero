/* zero assembler [virtual] machine interface */

#include <zas/conf.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <ctype.h>
#include <zero/trix.h>
#include <zas/zas.h>
#include <v0/mach.h>
#include <v0/vm32.h>

extern struct v0    *v0vm;

static struct zasop *zasoptab[256];
static long          zasvalbits[V0_NINST_MAX / (sizeof(long) * CHAR_BIT)];
static long          zasrngbits[V0_NINST_MAX / (sizeof(long) * CHAR_BIT)];
static long          zasarginit = 0;

#define v0insthasval(code) (bitset(zasvalbits, code))
#define v0insthasrng(code) (bitset(zasrngbits, code))

/*
 * operation info structure addresses are stored in a multilevel table
 * - the top level table is indexed with the first byte of mnemonic and so on
 */
long
zasaddop(const char *str, struct zasop *op)
{
    long           key = *str++;
    struct zasop  *ptr1 = zasoptab[key];

    if (!ptr1) {
        ptr1 = calloc(256, sizeof(struct zasopinfo *));
        if (!ptr1) {
            fprintf(stderr, "failed to allocate operation table\n");

            return 0;
        }
        zasoptab[key] = ptr1;
    }

    return 1;
}

struct zasop *
zasfindop(const char *str)
{
    long          key = *str++;
    struct zasop *ptr = zasoptab[key];

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

zasuword_t
zasgetreg(char *str, zasword_t *retsize, char **retptr)
{
    zasuword_t reg = ~0;
#if (ZVMNEWHACKS)
    zasword_t  size = -1;
#endif

#if (ZVMDEBUG)
    fprintf(stderr, "getreg: %s\n", str);
#endif
#if (ZVMNEWHACKS)
    if (*str == 'r') {
        str++;
#if (ZVM32BIT)
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
#if (!ZVM32BIT)
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

    return reg;
}

struct zastoken *
zasprocinst(struct zastoken *token, zasmemadr_t adr,
            zasmemadr_t *retadr)
{
    zasop_t          *op = (zasop_t *)&v0vm->mem[adr];
    zasmemadr_t       opadr = rounduppow2(adr, 4);
    struct zastoken  *token1 = NULL;
    struct zastoken  *token2 = NULL;
    struct zastoken  *retval = NULL;
    struct zassymrec *sym;
    long              inst;
    zasword_t         val;
    uint8_t           narg = token->data.inst.narg;
//    uint8_t           len = token->data.inst.op == ZASNOP ? 1 : 4;
    uint8_t           len = sizeof(zasop_t);

    if (!zasarginit) {
        setbit(zasvalbits, v0mkopid(V0_SHIFT, V0_SHL));
        setbit(zasvalbits, v0mkopid(V0_SHIFT, V0_SHR));
        setbit(zasvalbits, v0mkopid(V0_SHIFT, V0_SAR));
        setbit(zasvalbits, v0mkopid(V0_IO, V0_IOR));
        setbit(zasvalbits, v0mkopid(V0_IO, V0_IOW));
        setbit(zasrngbits, v0mkopid(V0_STACK, V0_PSHM));
        setbit(zasrngbits, v0mkopid(V0_STACK, V0_POPM));
    }
    while (adr < opadr) {
#if (ZAS32BIT)
        *(uint32_t *)op = UINT32_C(~0);
#else
        *(uint64_t *)op = UINT32_C(~0);
#endif
        adr += sizeof(zasop_t);
        op++;
    }
//    adr = opadr;
#if (ZASDB)
    zasaddline(adr, token->data.inst.data, token->file, token->line);
#endif
    op->code = token->data.inst.op;
    if (op->code == ZASNOP) {
        retval = token->next;
        adr += sizeof(zasop_t);
    } else if (!narg) {
        /* FIXME: failure? */
        op->reg1 = 0;
        op->reg2 = 0;
        op->adr = 0;
        op->parm = 0;
        op->val = 0;
        retval = token->next;
    } else {
        token1 = token->next;
        zasfreetoken(token);
        if (token1) {
            switch(token1->type) {
                case ZASTOKENVALUE:
                    val = token1->data.value.val;
                    inst = v0getinst(op->code);
                    op->adr = V0_DIR_ADR;
                    if (v0insthasval(inst)) {
                        op->val = val;
                    } else {
                        switch (token->data.value.size) {
                            case 1:
                                op->arg[0].i8 = val;

                                break;
                            case 2:
                                op->arg[0].i16 = val;

                                break;
                            case 3:
                                op->arg[0].i32 = val;

                                break;
                            default:
                                fprintf(stderr,
                                        "invalid-size immediate value\n");

                                exit(1);
                        }
                        len += sizeof(zasarg_t);
                    }

                    break;
                case ZASTOKENREG:
                    op->adr = V0_REG_ADR;
                    op->reg1 = token1->data.reg;

                    break;
                case ZASTOKENSYM:
                    op->adr = V0_DIR_ADR;
                    sym = malloc(sizeof(struct zassymrec));
                    sym->name = strdup((char *)token1->data.sym.name);
                    sym->adr = (uintptr_t)&op->arg[0].adr;
                    zasqueuesym(sym);
                    len += sizeof(zasarg_t);

                    break;
                case ZASTOKENINDIR:
                    token1 = token1->next;
                    if (token1->type == ZASTOKENREG) {
                        op->adr = V0_REG_ADR;
                        op->reg1 = token1->data.reg;
                    } else {
                        fprintf(stderr,
                                "indirect addressing requires a register\n");

                        exit(1);
                    }

                    break;
                case ZASTOKENIMMED:
                    val = token1->data.value.val;
                    inst = v0getinst(op->code);
                    op->adr = V0_DIR_ADR;
                    if (v0insthasrng(inst)) {
                        v0setloval(op, val);
                        token1 = token1->next;
                        if (token1->type != ZASTOKENVALUE) {
                            fprintf(stderr,
                                    "invalid range argument\n");

                            exit(1);
                        }
                        val = token1->data.value.val;
                        v0sethival(op, val);
                    } else if (v0insthasval(op->code)) {
                        op->val = val;
                    } else {
                        switch (token->data.value.size) {
                            case 1:
                                op->arg[0].i8 = token1->data.value.val;

                                break;
                            case 2:
                                op->arg[0].i16 = token1->data.value.val;

                                break;
                            case 3:
                                op->arg[0].i32 = token1->data.value.val;

                                break;
                            default:
                                fprintf(stderr,
                                        "invalid-size immediate value\n");

                                exit(1);
                        }
                        len += sizeof(zasarg_t);
                    }

                    break;
                case ZASTOKENADR:
                    op->adr = V0_DIR_ADR;
                    sym = malloc(sizeof(struct zassymrec));
                    sym->name = strdup((char *)token1->data.sym.name);
                    sym->adr = (uintptr_t)&op->arg[0].adr;
                    zasqueuesym(sym);
                    len += sizeof(zasarg_t);

                    break;
                case ZASTOKENINDEX:
                    op->adr = V0_NDX_ADR;
                    op->reg1 = token1->data.ndx.reg;
                    op->arg[0].ndx = token1->data.ndx.val;
                    len += sizeof(zasarg_t);

                    break;
                default:
                    fprintf(stderr,
                            "invalid argument 1 of type %lx\n", token1->type);
                    zasprinttoken(token1);

                    exit(1);

                    break;
            }
            token2 = token1->next;
            zasfreetoken(token1);
            retval = token2;
        }
        if (narg == 2 && (token2)) {
            inst = v0getinst(op->code);
            switch(token2->type) {
                case ZASTOKENVALUE:
                    op->adr = V0_DIR_ADR;
                    if (inst != V0_STR) {
                        fprintf(stderr,
                                "invalid address operand for instruction\n");

                        exit(1);
                    }
                    op->arg[0].adr = token2->data.value.val;
                    len += sizeof(zasarg_t);

                    break;
                case ZASTOKENREG:
                    op->reg2 = token2->data.reg;

                    break;
                case ZASTOKENSYM:
                    op->adr = V0_DIR_ADR;
                    sym = malloc(sizeof(struct zassymrec));
                    sym->name = strdup((char *)token2->data.sym.name);
                    sym->adr = (uintptr_t)&op->arg[0].adr;
                    zasqueuesym(sym);
                    len += sizeof(zasarg_t);

                    break;
                case ZASTOKENINDIR:
                    token2 = token2->next;
                    if (token2->type == ZASTOKENREG) {
                        op->adr = V0_REG_ADR;
                        op->reg2 = token2->data.reg;
                    } else {
                        fprintf(stderr,
                                "indirect addressing requires a register\n");

                        exit(1);
                    }

                    break;
                case ZASTOKENIMMED:
                    op->adr = V0_DIR_ADR;
                    op->arg[0].adr = token2->val;
                    len += sizeof(zasarg_t);

                    break;
                case ZASTOKENADR:
                    op->adr = V0_DIR_ADR;
                    sym = malloc(sizeof(struct zassymrec));
                    sym->name = strdup((char *)token2->data.sym.name);
                    sym->adr = (uintptr_t)&op->arg[0].adr;
                    zasqueuesym(sym);
                    len += sizeof(zasarg_t);

                    break;
                case ZASTOKENINDEX:
                    op->adr = V0_NDX_ADR;
                    op->reg2 = token2->data.ndx.reg;
                    op->arg[0].ndx = token2->data.ndx.val;
                    len += sizeof(zasarg_t);

                    break;
                default:
                    fprintf(stderr,
                            "invalid argument 2 of type %lx\n", token2->type);
                    zasprinttoken(token2);

                    exit(1);

                    break;
            }
            retval = token2->next;
            zasfreetoken(token2);
        }
    }
//    op->size = len;
    *retadr = adr + len;

    return retval;
}

