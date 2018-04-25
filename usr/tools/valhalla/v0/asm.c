/* zero assembler [virtual] machine interface */

#include <vas/conf.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <ctype.h>
#include <stdint.h>
#include <zero/trix.h>
#include <vas/vas.h>
#include <v0/mach.h>
#include <v0/vm.h>
#include <v0/op.h>

extern struct v0 *v0vm;
struct vasop     *v0optab[256];
static long       vasopbits[V0_NINST_MAX];
static long       vasinitflg;

#define vasopisnop(op)    (*((uint32_t *)(op)) == V0_NOP)
//#define vasophasval(code) (bitset(v0valbits, code))

#define vassetnop(adr)    (*((uint32_t *)(op)) = V0_NOP)
#define vassetop(op, str, id, n, slen)                                  \
    do {                                                                \
        (op)->name = strndup(str, len);                                 \
        (op)->code = (id);                                              \
        (op)->narg = (n);                                               \
        (op)->len = (slen);                                             \
    } while (0)

/*
 * operation info structure addresses are stored in a multilevel table
 * - the top level table is indexed with the first byte of mnemonic and so on
 * TODO: more compact encoding of 6-bit characters + some flag bits in int32_t
 */
long
vasaddop(const char *str, uint8_t code, uint8_t narg)
{
    char         *cptr = (char *)str;
    long          key = 0;
    struct vasop *op;
    uint8_t       len;

    op = calloc(1, sizeof(struct vasop));
    if (!op) {

        return 0;
    }
    while (*cptr) {
        key = *cptr++;
    }
    if (key) {
        key &= 0xff;
        len = cptr - str;
        vassetop(op, str, code, narg, len);
        op->next = v0optab[key];
        v0optab[key] = op;
    }

    return key;
}

static struct vasop *
vasfindop(char *str)
{
    char         *cptr = (char *)str;
    struct vasop *op = NULL;
    long          key = 0;

    while (*cptr) {
        key = *cptr++;
    }
    if (key) {
        key &= 0xff;
        op = v0optab[key];
        while ((op) && strcmp(op->name, str)) {
            op = op->next;
        }
    }

    return op;
}

struct vasop *
vasgetop(char *str, char **retptr)
{
    struct vasop *op = NULL;

    op = vasfindop(str);
#if (VASDEBUG)
    fprintf(stderr, "getop: %s\n", str);
#endif
    if (op) {
        str += op->len;
        *retptr = str;
    }

    return op;
}

vasuword
vasgetreg(char *str, vasword *retsize, char **retptr)
{
    vasuword reg = ~0;
    vasword  size = 0;

#if (VASDEBUG)
    fprintf(stderr, "getreg: %s\n", str);
#endif
    if (*str == 'r') {
        str++;
        size = 4;
    } else if (*str == 'b') {
        str++;
        size = 1;
    } else if (*str == 'w') {
        str++;
        size = 2;
    } else if (*str == 'l') {
        str++;
        size = 4;
#if (!V032BIT)
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

    return reg;
}

static void
vasinitops(void)
{
    if (!vasinitflg) {
        v0initopbits(vasopbits);
        vasinitflg = 1;
    }

    return;
}

void
vasdisasm(struct v0 *vm, struct v0op *op, v0reg pc)
{
    fprintf(stderr, "%p:%p\t%s @ 0x%x\t", vm, op, v0opnametab[op->code], pc);
    fprintf(stderr, "r1 = %x, r2 = %x, a = %x, p = %x, v = %x\n",
            op->reg1, op->reg2, op->adr, op->parm, op->val);

    return;
}

struct vastoken *
vasprocop(struct vastoken *token, v0memadr adr,
          v0memadr *retadr)
{
    struct v0op      *op = (struct v0op *)&v0vm->mem[adr];
    v0memadr          opadr = rounduppow2(adr, 4);
    struct vastoken  *token1 = NULL;
    struct vastoken  *token2 = NULL;
    struct vastoken  *retval = NULL;
    struct vassymrec *sym;
    long              havearg = 0;
    vasword           val;
    uint8_t           narg = token->data.inst.narg;
//    uint8_t           len = token->data.op.op == V0NOP ? 1 : 4;
    uint8_t           len = 1;

    if (!vasinitflg) {
        vasinitops();
    }
    while (adr < opadr) {
#if (V032BIT)
        *(uint32_t *)op = UINT32_C(~0);
#else
        *(uint64_t *)op = UINT32_C(~0);
#endif
        adr += sizeof(struct v0op);
        op++;
    }
//    adr = opadr;
#if (VASDB)
    vasaddline(adr, token->data.inst.data, token->file, token->line);
#endif
    op->code = token->data.inst.code;
    if (vasopisnop(op)) {
        retval = token->next;
        vassetnop(adr);
        adr += sizeof(struct v0op);
    } else if (!narg) {
        /* FIXME: failure? */
        retval = token->next;
        adr += sizeof(struct v0op);
    } else {
        token1 = token->next;
        vasfreetoken(token);
        if (token1) {
            switch(token1->type) {
                case VASTOKENVALUE:
                case VASTOKENIMMED:
                    val = token1->data.value.val;
                    if (vasopbits[op->code] & V0_I_ARG) {
                        op->adr = V0_IMM_ADR;
                        op->val = val;
                        adr += sizeof(struct v0op);
                    } else {
                        op->adr = V0_DIR_ADR;
                        havearg = 1;
                        adr += sizeof(struct v0op) + sizeof(union v0oparg);
                        switch (token->data.value.size) {
                            case 1:
                                op->arg[0].i32 = val;

                                break;
                            case 2:
                                op->arg[0].i32 = val;

                                break;
                            case 3:
                                op->arg[0].i32 = val;

                                break;
                            default:
                                fprintf(stderr,
                                        "invalid-size immediate value\n");

                                exit(1);
                        }
                        len++;
                        op->parm = len;
                    }

                    break;
                case VASTOKENREG:
                    op->adr = V0_REG_ADR;
                    op->reg1 = token1->data.reg;

                    break;
                case VASTOKENSYM:
                case VASTOKENADR:
                    op->adr = V0_DIR_ADR;
                    sym = malloc(sizeof(struct vassymrec));
                    sym->name = strdup((char *)token1->data.sym.name);
                    sym->adr = (uintptr_t)&op->arg[0].adr;
                    vasqueuesym(sym);
                    len++;

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
                case VASTOKENINDEX:
                    op->adr = V0_NDX_ADR;
                    op->reg1 = token1->data.ndx.reg;
                    op->arg[0].ofs = token1->data.ndx.val;
                    len++;

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
            if (token2->type == VASTOKENREG) {
                op->reg2 = token2->data.reg;
            } else if (havearg) {
                fprintf(stderr,
                        "too many non-register arguments\n");

                exit(1);
            } else {
                switch(token2->type) {
                    case VASTOKENVALUE:
                    case VASTOKENIMMED:
                        val = token2->data.value.val;
                        if (vasopbits[op->code] & V0_I_ARG) {
                            op->adr = V0_IMM_ADR;
                            op->val = val;
                        } else {
                            op->adr = V0_DIR_ADR;
                            havearg = 1;
                            switch (token->data.value.size) {
                                case 1:
                                    op->arg[0].i32 = val;

                                    break;
                                case 2:
                                    op->arg[0].i32 = val;

                                    break;
                                case 3:
                                    op->arg[0].i32 = val;

                                    break;
                                default:
                                    fprintf(stderr,
                                            "invalid-size immediate value\n");

                                    exit(1);
                            }
                            len++;
                            op->parm = len;
                        }

                        break;
                    case VASTOKENSYM:
                    case VASTOKENADR:
                        op->adr = V0_DIR_ADR;
                        sym = malloc(sizeof(struct vassymrec));
                        sym->name = strdup((char *)token2->data.sym.name);
                        sym->adr = (uintptr_t)&op->arg[0].adr;
                        vasqueuesym(sym);
                        len++;

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
                    case VASTOKENINDEX:
                        op->adr = V0_NDX_ADR;
                        op->reg2 = token2->data.ndx.reg;
                        op->arg[0].ofs = token2->data.ndx.val;
                        len++;

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
    }
    op->parm = len;
#if (V032BIT)
    len <<= 2;
#else
    len <<= 3;
#endif
    adr += len;
    *retadr = adr;

    return retval;
}

#if 0
static struct vastoken *
vasprocinst(struct vastoken *token, vasmemadr adr,
            vasmemadr *retadr)
{
    struct v0op      *op = NULL;
    //    vasmemadr         opadr = rounduppow2(adr, 4);
    struct vastoken  *token1 = NULL;
    struct vastoken  *token2 = NULL;
    struct vastoken  *retval = NULL;
    struct vassymrec *sym;
    uint8_t           narg = token->data.inst.narg;
    //    uint8_t           len = token->data.inst.op == VASNOP ? 1 : 4;
    uint8_t           len = 4;

    //    adr = opadr;
#if (VASDB)
    vasaddline(adr, token->data.inst.data, token->file, token->line);
#endif
    op = (struct v0op *)&v0vm->mem[adr];
    op->code = token->data.inst.code;
    if (op->code == V0_NOP) {
        vassetnop(op);
        retval = token->next;
    } else if (!narg) {
        op->reg1 = 0;
        op->reg2 = 0;
        op->parm = 0;
        retval = token->next;
    } else {
        token1 = token->next;
        vasfreetoken(token);
        if (token1) {
            switch(token1->type) {
                case VASTOKENVALUE:
                    if (vasophasval(op->code)) {
                        op->val = token1->data.value.val;
                    } else {
                        op->adr = V0_DIR_ADR;
                        op->arg[0].val = token1->data.value.val;
                        len += sizeof(vasword);
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
                    len += sizeof(uintptr_t);

                    break;
                case VASTOKENINDIR:
                    token1 = token1->next;
                    if (token1->type == VASTOKENREG) {
                        op->adr = V0_NDX_ADR;
                        op->reg1 = token1->data.reg;
                    } else {
                        fprintf(stderr, "indirect addressing requires a register\n");

                        exit(1);
                    }

                    break;
                case VASTOKENIMMED:
                    if (vasophasval(op->code)) {
                        op->adr = V0_IMM_ADR;
                        op->val = token1->val;
                    } else {
                        op->adr = V0_DIR_ADR;
                        op->arg[0].val = token1->val;
                        len += sizeof(vasword);
                    }

                    break;
                case VASTOKENADR:
                    op->adr = V0_DIR_ADR;
                    sym = malloc(sizeof(struct vassymrec));
                    sym->name = strdup((char *)token1->data.sym.name);
                    sym->adr = (uintptr_t)&op->arg[0].adr;
                    vasqueuesym(sym);
                    len += sizeof(uintptr_t);

                    break;
                case VASTOKENINDEX:
                    op->adr = V0_NDX_ADR;
                    op->reg1 = token1->data.ndx.reg;
                    op->arg[0].ofs = token1->data.ndx.val;
                    len += sizeof(vasword);

                    break;
                default:
                    fprintf(stderr, "invalid argument 1 of type %lx\n", token1->type);
                    printtoken(token1);

                    exit(1);

                    break;
            }
            token2 = token1->next;
            vasfreetoken(token1);
            retval = token2;
        }
        if (narg == 1) {
            op->adr = 0;
        } else if (narg == 2 && (token2)) {
            switch(token2->type) {
                case VASTOKENVALUE:
                    op->adr = V0_DIR_ADR;
                    op->arg[0].val = token2->data.value.val;
                    len += sizeof(vasword);

                    break;
                case VASTOKENREG:
                    op->adr = V0_REG_ADR;
                    op->reg2 = token2->data.reg;

                    break;
                case VASTOKENSYM:
                    op->adr = V0_DIR_ADR;
                    sym = malloc(sizeof(struct vassymrec));
                    sym->name = strdup((char *)token2->data.sym.name);
                    sym->adr = (uintptr_t)&op->arg[0].adr;
                    vasqueuesym(sym);
                    len += sizeof(uintptr_t);

                    break;
                case VASTOKENINDIR:
                    token2 = token2->next;
                    if (token2->type == VASTOKENREG) {
                        op->adr = V0_REG_ADR;
                        op->reg2 = token2->data.reg;
                    } else {
                        fprintf(stderr, "indirect addressing requires a register\n");

                        exit(1);
                    }

                    break;
                case VASTOKENIMMED:
                    op->adr = V0_DIR_ADR;
                    op->arg[0].val = token2->val;
                    len += sizeof(vasword);

                    break;
                case VASTOKENADR:
                    op->adr = V0_DIR_ADR;
                    sym = malloc(sizeof(struct vassymrec));
                    sym->name = strdup((char *)token2->data.sym.name);
                    sym->adr = (uintptr_t)&op->arg[0].adr;
                    vasqueuesym(sym);
                    len += sizeof(uintptr_t);

                    break;
                case VASTOKENINDEX:
                    op->adr = V0_NDX_ADR;
                    op->reg2 = token2->data.ndx.reg;
                    op->arg[0].ofs = token2->data.ndx.val;
                    len += sizeof(vasword);

                    break;
                default:
                    fprintf(stderr, "invalid argument 2 of type %lx\n", token2->type);
                    printtoken(token2);

                    exit(1);

                    break;
            }
            retval = token2->next;
            vasfreetoken(token2);
        }
        op->size = len >> 2;
    }
    *retadr = adr + len;

    return retval;
}
#endif

