/* zero assembler [virtual] machine interface */

#include <vas/conf.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <ctype.h>
#include <zero/trix.h>
#include <vas/vas.h>
#include <v0/mach.h>
#include <v0/vm32.h>

extern struct v0    *v0vm;

static struct vasop *vasoptab[256];
static long          vasvalbits[V0_NINST_MAX / (sizeof(long) * CHAR_BIT)];
static long          vasrngbits[V0_NINST_MAX / (sizeof(long) * CHAR_BIT)];
static long          vasarginit = 0;

#define v0insthasval(code) (bitset(vasvalbits, code))
#define v0insthasrng(code) (bitset(vasrngbits, code))

/*
 * operation info structure addresses are stored in a multilevel table
 * - the top level table is indexed with the first byte of mnemonic and so on
 */
long
vasaddop(const char *str, struct vasop *op)
{
    long           key = *str++;
    struct vasop  *ptr1 = vasoptab[key];

    if (!ptr1) {
        ptr1 = calloc(256, sizeof(struct vasopinfo *));
        if (!ptr1) {
            fprintf(stderr, "failed to allocate operation table\n");

            return 0;
        }
        vasoptab[key] = ptr1;
    }

    return 1;
}

struct vasop *
vasfindop(const char *str)
{
    long          key = *str++;
    struct vasop *ptr = vasoptab[key];

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

vasuword_t
vasgetreg(char *str, vasword_t *retsize, char **retptr)
{
    vasuword_t reg = ~0;
#if (ZVMNEWHACKS)
    vasword_t  size = -1;
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

struct vastoken *
vasprocinst(struct vastoken *token, vasmemadr_t adr,
            vasmemadr_t *retadr)
{
    vasop_t          *op = (vasop_t *)&v0vm->mem[adr];
    vasmemadr_t       opadr = rounduppow2(adr, 4);
    struct vastoken  *token1 = NULL;
    struct vastoken  *token2 = NULL;
    struct vastoken  *retval = NULL;
    struct vassymrec *sym;
    long              inst;
    vasword_t         val;
    uint8_t           narg = token->data.inst.narg;
//    uint8_t           len = token->data.inst.op == VASNOP ? 1 : 4;
    uint8_t           len = sizeof(vasop_t);

    if (!vasarginit) {
        setbit(vasvalbits, v0mkopid(V0_SHIFT, V0_SHL));
        setbit(vasvalbits, v0mkopid(V0_SHIFT, V0_SHR));
        setbit(vasvalbits, v0mkopid(V0_SHIFT, V0_SAR));
        setbit(vasvalbits, v0mkopid(V0_IO, V0_IOR));
        setbit(vasvalbits, v0mkopid(V0_IO, V0_IOW));
        setbit(vasrngbits, v0mkopid(V0_STACK, V0_PSHM));
        setbit(vasrngbits, v0mkopid(V0_STACK, V0_POPM));
    }
    while (adr < opadr) {
#if (VAS32BIT)
        *(uint32_t *)op = UINT32_C(~0);
#else
        *(uint64_t *)op = UINT32_C(~0);
#endif
        adr += sizeof(vasop_t);
        op++;
    }
//    adr = opadr;
#if (VASDB)
    vasaddline(adr, token->data.inst.data, token->file, token->line);
#endif
    op->code = token->data.inst.op;
    if (op->code == VASNOP) {
        retval = token->next;
        adr += sizeof(vasop_t);
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
        vasfreetoken(token);
        if (token1) {
            switch(token1->type) {
                case VASTOKENVALUE:
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
                        len += sizeof(vasarg_t);
                    }

                    break;
                case VASTOKENREG:
                    op->adr = V0_REG_ADR;
                    op->reg1 = token1->data.reg;

                    break;
                case VASTOKENSYM:
                    op->adr = V0_DIR_ADR;
                    sym = malloc(sizeof(struct vassymrec));
                    sym->name = strdup((char *)token1->data.sym.name);
                    sym->adr = (uintptr_t)&op->arg[0].adr;
                    vasqueuesym(sym);
                    len += sizeof(vasarg_t);

                    break;
                case VASTOKENINDIR:
                    token1 = token1->next;
                    if (token1->type == VASTOKENREG) {
                        op->adr = V0_REG_ADR;
                        op->reg1 = token1->data.reg;
                    } else {
                        fprintf(stderr,
                                "indirect addressing requires a register\n");

                        exit(1);
                    }

                    break;
                case VASTOKENIMMED:
                    val = token1->data.value.val;
                    inst = v0getinst(op->code);
                    op->adr = V0_DIR_ADR;
                    if (v0insthasrng(inst)) {
                        v0setloval(op, val);
                        token1 = token1->next;
                        if (token1->type != VASTOKENVALUE) {
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
                        len += sizeof(vasarg_t);
                    }

                    break;
                case VASTOKENADR:
                    op->adr = V0_DIR_ADR;
                    sym = malloc(sizeof(struct vassymrec));
                    sym->name = strdup((char *)token1->data.sym.name);
                    sym->adr = (uintptr_t)&op->arg[0].adr;
                    vasqueuesym(sym);
                    len += sizeof(vasarg_t);

                    break;
                case VASTOKENINDEX:
                    op->adr = V0_NDX_ADR;
                    op->reg1 = token1->data.ndx.reg;
                    op->arg[0].ndx = token1->data.ndx.val;
                    len += sizeof(vasarg_t);

                    break;
                default:
                    fprintf(stderr,
                            "invalid argument 1 of type %lx\n", token1->type);
                    vasprinttoken(token1);

                    exit(1);

                    break;
            }
            token2 = token1->next;
            vasfreetoken(token1);
            retval = token2;
        }
        if (narg == 2 && (token2)) {
            inst = v0getinst(op->code);
            switch(token2->type) {
                case VASTOKENVALUE:
                    op->adr = V0_DIR_ADR;
                    if (inst != V0_STR) {
                        fprintf(stderr,
                                "invalid address operand for instruction\n");

                        exit(1);
                    }
                    op->arg[0].adr = token2->data.value.val;
                    len += sizeof(vasarg_t);

                    break;
                case VASTOKENREG:
                    op->reg2 = token2->data.reg;

                    break;
                case VASTOKENSYM:
                    op->adr = V0_DIR_ADR;
                    sym = malloc(sizeof(struct vassymrec));
                    sym->name = strdup((char *)token2->data.sym.name);
                    sym->adr = (uintptr_t)&op->arg[0].adr;
                    vasqueuesym(sym);
                    len += sizeof(vasarg_t);

                    break;
                case VASTOKENINDIR:
                    token2 = token2->next;
                    if (token2->type == VASTOKENREG) {
                        op->adr = V0_REG_ADR;
                        op->reg2 = token2->data.reg;
                    } else {
                        fprintf(stderr,
                                "indirect addressing requires a register\n");

                        exit(1);
                    }

                    break;
                case VASTOKENIMMED:
                    op->adr = V0_DIR_ADR;
                    op->arg[0].adr = token2->val;
                    len += sizeof(vasarg_t);

                    break;
                case VASTOKENADR:
                    op->adr = V0_DIR_ADR;
                    sym = malloc(sizeof(struct vassymrec));
                    sym->name = strdup((char *)token2->data.sym.name);
                    sym->adr = (uintptr_t)&op->arg[0].adr;
                    vasqueuesym(sym);
                    len += sizeof(vasarg_t);

                    break;
                case VASTOKENINDEX:
                    op->adr = V0_NDX_ADR;
                    op->reg2 = token2->data.ndx.reg;
                    op->arg[0].ndx = token2->data.ndx.val;
                    len += sizeof(vasarg_t);

                    break;
                default:
                    fprintf(stderr,
                            "invalid argument 2 of type %lx\n", token2->type);
                    vasprinttoken(token2);

                    exit(1);

                    break;
            }
            retval = token2->next;
            vasfreetoken(token2);
        }
    }
//    op->size = len;
    *retadr = adr + len;

    return retval;
}

